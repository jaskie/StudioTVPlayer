#include "../pch.h"
#include "DecklinkInput.h"
#include "DecklinkUtils.h"
#include "DecklinkInputSynchroProvider.h"
#include "../Core/Channel.h"
#include "../Core/VideoFormat.h"
#include "../FieldOrder.h"
#include "../Preview/InputPreview.h"
#include "../Common/Debug.h"


namespace TVPlayR {
	namespace Decklink {

		static bool IsAutodetectionSupprted(IDeckLink* decklink)
		{
			CComQIPtr<IDeckLinkAttributes> attributes_(decklink);
			BOOL format_auto_detection = false;
			if (FAILED(attributes_->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &format_auto_detection)))
				format_auto_detection = false;
			return format_auto_detection;
		}

		struct DecklinkInput::implementation: public IDeckLinkInputCallback, Common::DebugTarget
		{
			CComQIPtr<IDeckLinkInput>									input_;
			const bool													is_autodetection_supported_;
			const bool													is_wide_;
			const bool													capture_video_;
			std::vector<std::unique_ptr<DecklinkInputSynchroProvider>>	channel_prividers_;
			std::vector<std::shared_ptr<Preview::InputPreview>>			previews_;
			BMDTimeValue												frame_duration_ = 0LL;
			BMDTimeScale												time_scale_ = 1LL;
			const int													audio_channels_count_;
			const TVPlayR::DecklinkTimecodeSource						timecode_source_;
			Core::VideoFormat											current_format_;
			std::mutex													channel_list_mutex_;
			FORMAT_CALLBACK												format_changed_callback_ = nullptr;
			TIME_CALLBACK												frame_played_callback_ = nullptr;


			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType initial_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video)
				: Common::DebugTarget(false, "Decklink input")
				, input_(decklink)
				, is_wide_(!(initial_format == Core::VideoFormatType::ntsc || initial_format == Core::VideoFormatType::pal))
				, is_autodetection_supported_(IsAutodetectionSupprted(decklink))
				, audio_channels_count_(audio_channels_count)
				, timecode_source_(timecode_source)
				, capture_video_(capture_video)
				, current_format_(initial_format)
			{
				BMDDisplayMode mode = GetDecklinkDisplayMode(initial_format);
				BMDDisplayModeSupport support;
				IDeckLinkDisplayMode* initialDisplayMode = nullptr;
				if (FAILED(input_->DoesSupportVideoMode(mode, BMDPixelFormat::bmdFormat8BitYUV, bmdVideoInputFlagDefault, &support, &initialDisplayMode)))
					THROW_EXCEPTION("DecklinkInput: DoesSupportVideoMode failed");				
				if (support == BMDDisplayModeSupport::bmdDisplayModeNotSupported)
					THROW_EXCEPTION("DecklinkInput: Display mode not supported");
				input_->SetCallback(this);
				OpenInput(initialDisplayMode);
			}

			implementation::~implementation()
			{
				CloseInput();
				input_->SetCallback(NULL);
			}

			void OpenInput(IDeckLinkDisplayMode* displayMode)
			{
				if (FAILED(displayMode->GetFrameRate(&frame_duration_, &time_scale_)))
					THROW_EXCEPTION("DecklinkInput: GetFrameRate failed");
				if (FAILED(input_->EnableVideoInput(displayMode->GetDisplayMode(), BMDPixelFormat::bmdFormat8BitYUV, is_autodetection_supported_ ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault)))
					THROW_EXCEPTION("DecklinkInput: EnableVideoInput failed");
				if (audio_channels_count_ > 0)
				{
					if (FAILED(input_->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType32bitInteger, audio_channels_count_)))
						THROW_EXCEPTION("DecklinkInput: EnableAudioInput failed");
				}
				if (FAILED(input_->StartStreams()))
					THROW_EXCEPTION("DecklinkInput: StartStreams failed");
			}

			void CloseInput()
			{
				if (!input_)
					return;
				if (FAILED(input_->StopStreams()))
					THROW_EXCEPTION("DecklinkInput: StopStreams failed");
				if (FAILED(input_->DisableAudioInput()))
					THROW_EXCEPTION("DecklinkInput: DisableAudioInput failed");
				if (FAILED(input_->DisableVideoInput()))
					THROW_EXCEPTION("DecklinkInput: DisableVideoInput failed");
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override
			{
				if (notificationEvents & bmdVideoInputDisplayModeChanged)
				{
					CloseInput();
					current_format_ = BMDDisplayModeToVideoFormatType(newDisplayMode->GetDisplayMode(), is_wide_);
					if (current_format_.type() == Core::VideoFormatType::invalid)
						return S_OK;
					for (auto& provider : channel_prividers_)
						provider->Reset(current_format_.FrameRate().av());
					OpenInput(newDisplayMode);
					if (format_changed_callback_)
						format_changed_callback_(BMDDisplayModeToVideoFormatType(newDisplayMode->GetDisplayMode(), is_wide_));
				}
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				if (current_format_.type() == Core::VideoFormatType::invalid)
					return S_OK;

				std::shared_ptr<AVFrame> video = AVFrameFromDecklinkVideo(videoFrame, timecode_source_, current_format_, time_scale_);
				std::shared_ptr<AVFrame> audio = AVFrameFromDecklinkAudio(audioPacket, audio_channels_count_, AV_SAMPLE_FMT_S32, bmdAudioSampleRate48kHz);
				int64_t timecode = TimeFromDeclinkTimecode(videoFrame, timecode_source_, current_format_.FrameRate());
				for (auto& provider : channel_prividers_)
					provider->Push(video, audio, timecode);
				for (auto& preview : previews_)
					preview->Push(video);
				if (frame_played_callback_)
					frame_played_callback_(timecode);
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			virtual ULONG STDMETHODCALLTYPE		AddRef() override { return 1; }
			virtual ULONG STDMETHODCALLTYPE		Release() override { return 1; }

			bool IsAddedToChannel(const Core::Channel& channel)
			{
				return std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& provider) { return &provider->Channel() == &channel; }) != channel_prividers_.end();
			}

			void AddToChannel(const Core::Channel& channel)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				if (!IsAddedToChannel(channel))
				{
					std::unique_ptr<DecklinkInputSynchroProvider> provider = std::make_unique<DecklinkInputSynchroProvider>(channel, timecode_source_, capture_video_);
					provider->Reset(current_format_.FrameRate().av());
					channel_prividers_.emplace_back(std::move(provider));
				}
			}

			void RemoveFromChannel(const Core::Channel& channel)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				auto provider = std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& p) { return &p->Channel() == &channel; });
				if (provider == channel_prividers_.end())
					return;
				channel_prividers_.erase(provider);
			}

			void AddPreview(std::shared_ptr<Preview::InputPreview>& preview)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				previews_.push_back(preview);
			}

			void RemovePreview(std::shared_ptr<Preview::InputPreview>& preview)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				previews_.erase(std::remove(previews_.begin(), previews_.end(), preview), previews_.end());
			}

			int GetWidth() const
			{
				return current_format_.width();
			}

			int GetHeight() const
			{
				return current_format_.height();
			}

			int GetAudioChannelsCount() const
			{
				return audio_channels_count_;
			}

			FFmpeg::AVSync PullSync(const Core::Channel& channel, int audio_samples_count)
			{
				auto provider = std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& p) { return &p->Channel() == &channel; });
				if (provider == channel_prividers_.end())
					return FFmpeg::AVSync();
				return (*provider)->PullSync(audio_samples_count);
			}

			TVPlayR::FieldOrder GetFieldOrder() const
			{
				return current_format_.field_order();
			}

		};

		DecklinkInput::DecklinkInput(IDeckLink* decklink, Core::VideoFormatType initial_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video)
			: impl_(std::make_unique<implementation>(decklink, initial_format, audio_channels_count, timecode_source, capture_video))
		{ }
		
		DecklinkInput::~DecklinkInput()	{ }
		
		FFmpeg::AVSync DecklinkInput::PullSync(const Core::Channel& channel, int audio_samples_count) { return impl_->PullSync(channel, audio_samples_count); }

		bool DecklinkInput::IsAddedToChannel(const Core::Channel& channel) { return impl_->IsAddedToChannel(channel); }
		void DecklinkInput::AddToChannel(const Core::Channel& channel) { impl_->AddToChannel(channel); }
		void DecklinkInput::RemoveFromChannel(const Core::Channel& channel) { impl_->RemoveFromChannel(channel); }
		void DecklinkInput::AddPreview(std::shared_ptr<Preview::InputPreview> preview) { impl_->AddPreview(preview); }
		void DecklinkInput::RemovePreview(std::shared_ptr<Preview::InputPreview> preview) { impl_->RemovePreview(preview); }
		void DecklinkInput::Play() { }
		void DecklinkInput::Pause()	{ }
		bool DecklinkInput::IsPlaying() const { return true; }
		int DecklinkInput::GetWidth() const { return impl_->GetWidth(); }
		int DecklinkInput::GetHeight() const { return impl_->GetHeight(); }
		TVPlayR::FieldOrder DecklinkInput::GetFieldOrder() { return impl_->GetFieldOrder(); }
		int DecklinkInput::GetAudioChannelCount() { return impl_->GetAudioChannelsCount(); }
		bool DecklinkInput::HaveAlphaChannel() const { return false; }
		void DecklinkInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
		void DecklinkInput::SetFormatChangedCallback(FORMAT_CALLBACK format_changed_callback) { impl_->format_changed_callback_ = format_changed_callback; }
	}
}