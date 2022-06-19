#include "../pch.h"
#include "DecklinkInput.h"
#include "DecklinkUtils.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkInputSynchroProvider.h"
#include "../Core/Player.h"
#include "../Core/VideoFormat.h"
#include "../Core/AVSync.h"
#include "../FieldOrder.h"
#include "../Core/OutputDevice.h"

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
			const BMDAudioSampleType									AUDIO_SAMPLE_TYPE = BMDAudioSampleType::bmdAudioSampleType32bitInteger;
			CComQIPtr<IDeckLinkInput>									input_;
			const bool													is_autodetection_supported_;
			const bool													is_wide_;
			const bool													capture_video_;
			std::vector<std::unique_ptr<DecklinkInputSynchroProvider>>	player_providers_;
			std::vector<std::shared_ptr<Core::OutputSink>>				previews_;
			BMDTimeScale												time_scale_ = 1LL;
			std::int64_t												last_frame_time_ = 0;
			const int													audio_channels_count_;
			const TVPlayR::DecklinkTimecodeSource						timecode_source_;
			Core::VideoFormat											current_format_;
			std::mutex													channel_list_mutex_;
			FORMAT_CALLBACK												format_changed_callback_ = nullptr;
			TIME_CALLBACK												frame_played_callback_ = nullptr;


			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType initial_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video)
				: Common::DebugTarget(Common::DebugSeverity::info, "Decklink input")
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
				BMDTimeValue frame_duration;
				if (FAILED(displayMode->GetFrameRate(&frame_duration, &time_scale_)))
					THROW_EXCEPTION("DecklinkInput: GetFrameRate failed");
				if (FAILED(input_->EnableVideoInput(displayMode->GetDisplayMode(), BMDPixelFormat::bmdFormat8BitYUV, is_autodetection_supported_ ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault)))
					THROW_EXCEPTION("DecklinkInput: EnableVideoInput failed");
				if (audio_channels_count_ > 0)
				{
					if (FAILED(input_->EnableAudioInput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, AUDIO_SAMPLE_TYPE, audio_channels_count_)))
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

			STDMETHODIMP VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override
			{
				if (notificationEvents & bmdVideoInputDisplayModeChanged)
				{
					CloseInput();
					current_format_ = BMDDisplayModeToVideoFormatType(newDisplayMode->GetDisplayMode(), is_wide_);
					if (current_format_.type() == Core::VideoFormatType::invalid)
						return S_OK;
					for (auto& provider : player_providers_)
						provider->Reset(current_format_.FrameRate().av());
					OpenInput(newDisplayMode);
					if (format_changed_callback_)
						format_changed_callback_(BMDDisplayModeToVideoFormatType(newDisplayMode->GetDisplayMode(), is_wide_));
				}
				return S_OK;
			}

			STDMETHODIMP VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				if (current_format_.type() == Core::VideoFormatType::invalid)
					return S_OK;
				BMDTimeValue bmd_time, bmd_duration;
				if (SUCCEEDED(videoFrame->GetStreamTime(&bmd_time, &bmd_duration, time_scale_)))
					last_frame_time_ = bmd_time * AV_TIME_BASE / time_scale_;
				Core::AVSync sync(
					AVFrameFromDecklinkAudio(audioPacket, audio_channels_count_, AUDIO_SAMPLE_TYPE, BMDAudioSampleRate::bmdAudioSampleRate48kHz),
					AVFrameFromDecklinkVideo(videoFrame, timecode_source_, current_format_, time_scale_),
					Core::FrameTimeInfo {
						TimeFromDeclinkTimecode(videoFrame, timecode_source_, current_format_.FrameRate()),
						last_frame_time_,
						AV_NOPTS_VALUE }
					);
				for (auto& provider : player_providers_)
					provider->Push(sync);
				for (auto& preview : previews_)
					preview->Push(sync);
				if (frame_played_callback_)
					frame_played_callback_(sync.TimeInfo);
				return S_OK;
			}

			STDMETHODIMP						QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			virtual ULONG STDMETHODCALLTYPE		AddRef() override { return 1; }
			virtual ULONG STDMETHODCALLTYPE		Release() override { return 1; }

			bool IsAddedToPlayer(const Core::Player& player)
			{
				return std::find_if(player_providers_.begin(), player_providers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& provider) { return &provider->Player() == &player; }) != player_providers_.end();
			}

			void AddToPlayer(const Core::Player& player)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				if (!IsAddedToPlayer(player))
				{
					std::unique_ptr<DecklinkInputSynchroProvider> provider = std::make_unique<DecklinkInputSynchroProvider>(player, timecode_source_, capture_video_, audio_channels_count_);
					provider->Reset(current_format_.FrameRate().av());
					player_providers_.emplace_back(std::move(provider));
				}
			}

			void RemoveFromPlayer(const Core::Player& player)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				auto provider = std::find_if(player_providers_.begin(), player_providers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& p) { return &p->Player() == &player; });
				if (provider == player_providers_.end())
					return;
				player_providers_.erase(provider);
			}

			void AddPreview(std::shared_ptr<Core::OutputSink> preview)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				previews_.push_back(preview);
			}

			void RemovePreview(std::shared_ptr<Core::OutputSink> preview)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				auto item = std::find_if(previews_.begin(), previews_.end(), [&](const std::shared_ptr<Core::OutputSink>& p) { return p == preview; });
				if (item == previews_.end())
					return;
				previews_.erase(item);
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

			Core::AVSync PullSync(const Core::Player& player, int audio_samples_count)
			{
				auto provider = std::find_if(player_providers_.begin(), player_providers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& p) { return &p->Player() == &player; });
				if (provider == player_providers_.end())
					return Core::AVSync();
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
		
		Core::AVSync DecklinkInput::PullSync(const Core::Player& player, int audio_samples_count) { return impl_->PullSync(player, audio_samples_count); }

		bool DecklinkInput::IsAddedToPlayer(const Core::Player& player) { return impl_->IsAddedToPlayer(player); }
		void DecklinkInput::AddToPlayer(const Core::Player& player) { impl_->AddToPlayer(player); }
		void DecklinkInput::RemoveFromPlayer(const Core::Player& player) { impl_->RemoveFromPlayer(player); }
		void DecklinkInput::AddPreview(std::shared_ptr<Core::OutputSink> preview) { impl_->AddPreview(preview); }
		void DecklinkInput::RemovePreview(std::shared_ptr<Core::OutputSink> preview) { impl_->RemovePreview(preview); }
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