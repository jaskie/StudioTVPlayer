#include "../pch.h"
#include "DecklinkInput.h"
#include "DecklinkUtils.h"
#include "DecklinkSynchroProvider.h"
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

		struct DecklinkInput::implementation: public IDeckLinkInputCallback, Common::DebugTarget<false>
		{
			CComQIPtr<IDeckLinkInput>	input_;
			Core::VideoFormat requested_format_;
			CComPtr<IDeckLinkDisplayMode> current_display_mode_;
			const bool is_autodetection_supported_;
			const bool is_wide_;
			std::vector<DecklinkSynchroProvider> channel_prividers_;

			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType requestedFormat)
				: input_(decklink)
				, requested_format_(requestedFormat)
				, is_wide_(!(requestedFormat == Core::VideoFormatType::ntsc || requestedFormat == Core::VideoFormatType::pal))
				, is_autodetection_supported_(IsAutodetectionSupprted(decklink))
			{
				BMDDisplayMode mode = GetDecklinkDisplayMode(requestedFormat);
				BMDDisplayModeSupport support;
				if (FAILED(input_->DoesSupportVideoMode(mode, BMDPixelFormat::bmdFormat8BitYUV, bmdVideoInputFlagDefault, &support, &current_display_mode_)))
					THROW_EXCEPTION("DecklinkInput: DoesSupportVideoMode failed");				
				if (support == BMDDisplayModeSupport::bmdDisplayModeNotSupported)
					THROW_EXCEPTION("DecklinkInput: Display mode not supported");
				OpenInput();
			}

			implementation::~implementation()
			{
				CloseInput();
			}

			void OpenInput()
			{
				if (FAILED(input_->EnableVideoInput(current_display_mode_->GetDisplayMode(), BMDPixelFormat::bmdFormat8BitYUV, is_autodetection_supported_ ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault)))
					THROW_EXCEPTION("DecklinkInput: EnableVideoInput failed");
				if (FAILED(input_->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType32bitInteger, 2)))
					THROW_EXCEPTION("DecklinkInput: EnableAudioInput failed");
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
					current_display_mode_ = newDisplayMode;
					CloseInput();
					OpenInput();
				}
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override
			{
				for (auto& provider : channel_prividers_)
					provider.Push(videoFrame, current_display_mode_->GetFieldDominance(), audioPacket);
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			virtual ULONG STDMETHODCALLTYPE		AddRef() override { return 1; }
			virtual ULONG STDMETHODCALLTYPE		Release() override { return 1; }

			bool IsAddedToChannel(const Core::Channel& channel)
			{
				return std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const DecklinkSynchroProvider& provider) { return provider.Channel() == &channel; }) != channel_prividers_.end();
			}

			void AddToChannel(const Core::Channel& channel)
			{
				if (!IsAddedToChannel(channel))
					channel_prividers_.emplace_back(&channel);
			}

			void RemoveFromChannel(const Core::Channel& channel)
			{
				auto provider = std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const DecklinkSynchroProvider& p) { return p.Channel() == &channel; });
				if (provider == channel_prividers_.end())
					return;
				channel_prividers_.erase(provider);
			}

			int GetWidth() const
			{
				return requested_format_.width();
			}

			int GetHeight() const
			{
				return requested_format_.height();
			}

			FFmpeg::AVSync PullSync(const Core::Channel& channel, int audio_samples_count)
			{
				auto provider = std::find_if(channel_prividers_.begin(), channel_prividers_.end(), [&](const DecklinkSynchroProvider& p) { return p.Channel() == &channel; });
				if (provider == channel_prividers_.end())
					return FFmpeg::AVSync();
				return provider->PullSync(audio_samples_count);
			}
		};

		DecklinkInput::DecklinkInput(IDeckLink* decklink, Core::VideoFormatType requestedFormat)
			: impl_(std::make_unique<implementation>(decklink, requestedFormat))
		{ }
		
		DecklinkInput::~DecklinkInput()	{ }
		
		FFmpeg::AVSync DecklinkInput::PullSync(const Core::Channel& channel, int audio_samples_count) { return impl_->PullSync(channel, audio_samples_count); }

		bool DecklinkInput::IsAddedToChannel(const Core::Channel& channel) { return impl_->IsAddedToChannel(channel); }
		void DecklinkInput::AddToChannel(const Core::Channel& channel) { impl_->AddToChannel(channel); }
		void DecklinkInput::RemoveFromChannel(const Core::Channel& channel) { impl_->RemoveFromChannel(channel); }
		void DecklinkInput::Play()
		{
		}
		void DecklinkInput::Pause()
		{
		}
		bool DecklinkInput::IsPlaying() const { return true; }
		
		int DecklinkInput::GetWidth() const { return impl_->GetWidth(); }
		int DecklinkInput::GetHeight() const { return impl_->GetHeight(); }

		Core::FieldOrder DecklinkInput::GetFieldOrder()
		{
			return Core::FieldOrder::unknown;
		}
		int DecklinkInput::GetAudioChannelCount()
		{
			return 0;
		}
		bool DecklinkInput::HaveAlphaChannel() const
		{
			return false;
		}

		void DecklinkInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback)
		{
		}
	}
}