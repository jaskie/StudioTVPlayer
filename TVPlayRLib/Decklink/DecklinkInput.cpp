#include "../pch.h"
#include "DecklinkInput.h"
#include "DecklinkUtils.h"
#include "DecklinkInputSynchroProvider.h"
#include "../Core/VideoFormat.h"
#include "../Core/FieldOrder.h"
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
			std::vector<std::unique_ptr<DecklinkInputSynchroProvider>>	channel_prividers_;
			BMDTimeValue												frame_duration_ = 0LL;
			BMDTimeScale												time_scale_ = 1LL;
			BMDFieldDominance											field_dominance_ = BMDFieldDominance::bmdUnknownFieldDominance;
			long														current_width_, current_height_ = 0L;
			const int													audio_channels_count_;
			const DecklinkTimecodeSource								timecode_source_;


			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType format, int audio_channels_count, DecklinkTimecodeSource timecode_source)
				: Common::DebugTarget(false, "Decklink input")
				, input_(decklink)
				, is_wide_(!(format == Core::VideoFormatType::ntsc || format == Core::VideoFormatType::pal))
				, is_autodetection_supported_(IsAutodetectionSupprted(decklink))
				, audio_channels_count_(audio_channels_count)
				, timecode_source_(timecode_source)
			{
				BMDDisplayMode mode = GetDecklinkDisplayMode(format);
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
				input_->SetCallback(NULL);
				CloseInput();
			}

			void OpenInput(IDeckLinkDisplayMode* displayMode)
			{
				field_dominance_ = displayMode->GetFieldDominance();
				current_width_ = displayMode->GetWidth();
				current_height_ = displayMode->GetHeight();
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
				for (auto& provider : channel_prividers_)
					provider->SetInputParameters(field_dominance_, time_scale_, frame_duration_);
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
					OpenInput(newDisplayMode);
				}
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override
			{
				for (auto& provider : channel_prividers_)
					provider->Push(videoFrame, audioPacket);
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
				if (!IsAddedToChannel(channel))
				{
					std::unique_ptr<DecklinkInputSynchroProvider> newProvider = std::make_unique<DecklinkInputSynchroProvider>(channel, timecode_source_);
					newProvider->SetInputParameters(field_dominance_, time_scale_, frame_duration_);
					channel_prividers_.emplace_back(std::move(newProvider));
				}
			}

			void RemoveFromChannel(const Core::Channel& channel)
			{
				auto provider = std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const std::unique_ptr<DecklinkInputSynchroProvider>& p) { return &p->Channel() == &channel; });
				if (provider == channel_prividers_.end())
					return;
				channel_prividers_.erase(provider);
			}

			int GetWidth() const
			{
				return current_width_;
			}

			int GetHeight() const
			{
				return current_height_;
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

			Core::FieldOrder GetFieldOrder() const
			{
				switch (field_dominance_)
				{
				case BMDFieldDominance::bmdLowerFieldFirst:
					return Core::FieldOrder::lower;
				case BMDFieldDominance::bmdUpperFieldFirst:
					return Core::FieldOrder::upper;
				case BMDFieldDominance::bmdProgressiveFrame:
				case BMDFieldDominance::bmdProgressiveSegmentedFrame:
					return Core::FieldOrder::progressive;
				default:
					return Core::FieldOrder::unknown;
				}
			}

		};

		DecklinkInput::DecklinkInput(IDeckLink* decklink, Core::VideoFormatType format, int audio_channels_count, DecklinkTimecodeSource timecode_source)
			: impl_(std::make_unique<implementation>(decklink, format, audio_channels_count, timecode_source))
		{ }
		
		DecklinkInput::~DecklinkInput()	{ }
		
		FFmpeg::AVSync DecklinkInput::PullSync(const Core::Channel& channel, int audio_samples_count) { return impl_->PullSync(channel, audio_samples_count); }

		bool DecklinkInput::IsAddedToChannel(const Core::Channel& channel) { return impl_->IsAddedToChannel(channel); }
		void DecklinkInput::AddToChannel(const Core::Channel& channel) { impl_->AddToChannel(channel); }
		void DecklinkInput::RemoveFromChannel(const Core::Channel& channel) { impl_->RemoveFromChannel(channel); }
		void DecklinkInput::AddPreview(std::shared_ptr<Preview::InputPreview> preview)
		{
		}
		void DecklinkInput::Play()
		{
		}
		void DecklinkInput::Pause()
		{
		}
		bool DecklinkInput::IsPlaying() const { return true; }
		
		int DecklinkInput::GetWidth() const { return impl_->GetWidth(); }
		int DecklinkInput::GetHeight() const { return impl_->GetHeight(); }

		Core::FieldOrder DecklinkInput::GetFieldOrder() { return impl_->GetFieldOrder(); }
		int DecklinkInput::GetAudioChannelCount() { return impl_->GetAudioChannelsCount(); }
		bool DecklinkInput::HaveAlphaChannel() const { return false; }
		void DecklinkInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback)
		{
		}
	}
}