#include "../pch.h"
#include "DecklinkInput.h"
#include "../Core/VideoFormat.h"

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

		struct DecklinkInput::implementation: public IDeckLinkInputCallback
		{
			CComQIPtr<IDeckLinkInput>	input_;
			CComQIPtr<IDeckLinkDisplayMode> current_display_mode_;
			const Core::VideoFormat format_;
			const bool is_autodetection_supported_;

			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType format)
				: input_(decklink)
				, format_(format)
				, is_autodetection_supported_(IsAutodetectionSupprted(decklink))
			{
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(
				/* [in] */ BMDVideoInputFormatChangedEvents notificationEvents,
				/* [in] */ IDeckLinkDisplayMode* newDisplayMode,
				/* [in] */ BMDDetectedVideoInputFormatFlags detectedSignalFlags) override
			{
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(
				/* [in] */ IDeckLinkVideoInputFrame* videoFrame,
				/* [in] */ IDeckLinkAudioInputPacket* audioPacket) override
			{
				return S_OK;
			}

			virtual HRESULT STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			virtual ULONG STDMETHODCALLTYPE		AddRef() override { return 1; }
			virtual ULONG STDMETHODCALLTYPE		Release() override { return 1; }

			void AddToChannel(const Core::Channel& channel)
			{

			}

			void RemoveFromChannel(const Core::Channel& channel)
			{

			}

		};

		DecklinkInput::DecklinkInput(IDeckLink* decklink, Core::VideoFormatType format)
			: impl_(std::make_unique<implementation>(decklink, format))
		{

		}
		
		DecklinkInput::~DecklinkInput()	{ }
		
		FFmpeg::AVSync DecklinkInput::PullSync(int audio_samples_count)
		{
			return FFmpeg::AVSync();
		}
		bool DecklinkInput::IsAddedToChannel(const Core::Channel& channel)
		{
			return false;
		}
		void DecklinkInput::AddToChannel(const Core::Channel& channel) { impl_->AddToChannel(channel); }
		void DecklinkInput::RemoveFromChannel(const Core::Channel& channel) { impl_->RemoveFromChannel(channel); }
		void DecklinkInput::Play()
		{
		}
		void DecklinkInput::Pause()
		{
		}
		bool DecklinkInput::IsPlaying() const
		{
			return false;
		}
		int DecklinkInput::GetWidth()
		{
			return 0;
		}
		int DecklinkInput::GetHeight()
		{
			return 0;
		}
		AVFieldOrder DecklinkInput::GetFieldOrder()
		{
			return AVFieldOrder();
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