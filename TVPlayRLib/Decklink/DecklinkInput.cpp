#include "../pch.h"
#include "DecklinkInput.h"
#include "DecklinkUtils.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkInputSynchroProvider.h"
#include "../Core/Player.h"
#include "../Core/VideoFormat.h"
#include "../Core/AVSync.h"
#include "../FieldOrder.h"
#include "../PixelFormat.h"
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Decklink {

		static bool IsFormatAutodetectionSupprted(IDeckLink* decklink)
		{
			CComQIPtr<IDeckLinkProfileAttributes> attributes_(decklink);
			BOOL format_auto_detection = false;
			if (FAILED(attributes_->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &format_auto_detection)))
				format_auto_detection = false;
			return format_auto_detection;
		}

		struct DecklinkInput::implementation: public IDeckLinkInputCallback, Common::DebugTarget
		{
			const BMDAudioSampleType									AUDIO_SAMPLE_TYPE = BMDAudioSampleType::bmdAudioSampleType32bitInteger;
			CComQIPtr<IDeckLinkInput>									input_;
			const bool													is_format_autodetection_supported_;
			const bool													is_wide_;
			const bool													capture_video_;
			const bool													format_autodetection_;
			const BMDPixelFormat										pixel_format_;
			std::vector<std::unique_ptr<DecklinkInputSynchroProvider>>	player_providers_;
			std::vector<std::shared_ptr<Core::OutputSink>>				output_sinks_;
			BMDTimeScale												time_scale_ = 1LL;
			std::int64_t												last_frame_time_ = 0;
			const int													audio_channels_count_;
			const TVPlayR::DecklinkTimecodeSource						timecode_source_;
			Core::VideoFormat											current_format_;
			FieldOrder													current_field_order_;
			AVRational													current_sar_;
			std::mutex													channel_list_mutex_;
			FORMAT_CALLBACK												format_changed_callback_ = nullptr;
			TIME_CALLBACK												frame_played_callback_ = nullptr;


			implementation::implementation(IDeckLink* decklink, Core::VideoFormatType initial_format, PixelFormat pixel_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video, bool format_autodetection)
				: Common::DebugTarget(Common::DebugSeverity::info, "Decklink input")
				, input_(decklink)
				, is_wide_(!(initial_format == Core::VideoFormatType::ntsc || initial_format == Core::VideoFormatType::pal))
				, is_format_autodetection_supported_(IsFormatAutodetectionSupprted(decklink))
				, audio_channels_count_(audio_channels_count)
				, timecode_source_(timecode_source)
				, capture_video_(capture_video)
				, current_format_(initial_format)
				, current_field_order_(current_format_.field_order())
				, current_sar_(current_format_.SampleAspectRatio().av())
				, format_autodetection_(format_autodetection)
				, pixel_format_(Decklink::BMDPixelFormatFromPixelFormat(pixel_format))
			{
				IDeckLinkDisplayMode* mode = FindMode(GetDecklinkDisplayMode(initial_format));
				OpenInput(mode, pixel_format_);
				if (FAILED(input_->SetCallback(this)))
					THROW_EXCEPTION("DecklinkInput: SetCallback failed");;
			}

			implementation::~implementation()
			{
				CloseInput();
				input_->SetCallback(NULL);
			}

			void OpenInput(IDeckLinkDisplayMode* displayMode, BMDPixelFormat pixel_format)
			{
				BMDTimeValue frame_duration;
				if (FAILED(displayMode->GetFrameRate(&frame_duration, &time_scale_)))
					THROW_EXCEPTION("DecklinkInput: GetFrameRate failed");
				if (FAILED(input_->EnableVideoInput(displayMode->GetDisplayMode(), pixel_format, is_format_autodetection_supported_ && format_autodetection_ ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault)))
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

			IDeckLinkDisplayMode* FindMode(BMDDisplayMode mode)
			{
				IDeckLinkDisplayModeIterator* displayModeIterator = nullptr;
				if (FAILED(input_->GetDisplayModeIterator(&displayModeIterator)))
					THROW_EXCEPTION("DecklinkInput: Display mode iterator creation failed");
				IDeckLinkDisplayMode* displayMode = nullptr;
				while (SUCCEEDED(displayModeIterator->Next(&displayMode)))
				{
					if (displayMode->GetDisplayMode() == mode)
						return displayMode;
				}
				THROW_EXCEPTION("DecklinkInput: Display mode not found");
			}

			STDMETHODIMP VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override
			{
				if (notificationEvents & _BMDVideoInputFormatChangedEvents::bmdVideoInputDisplayModeChanged)
				{
					CloseInput();
					current_format_ = BMDDisplayModeToVideoFormatType(newDisplayMode->GetDisplayMode(), is_wide_);
					if (current_format_.type() == Core::VideoFormatType::invalid)
						return S_OK;
					current_field_order_ = current_format_.field_order();
					current_sar_ = current_format_.SampleAspectRatio().av();
					for (auto& provider : player_providers_)
						provider->Reset(current_format_.FrameRate().av());
					OpenInput(newDisplayMode, pixel_format_);
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
				if (videoFrame == nullptr || audioPacket == nullptr)
					return E_FAIL;
				BMDTimeValue bmd_time, bmd_duration;
				if (SUCCEEDED(videoFrame->GetStreamTime(&bmd_time, &bmd_duration, time_scale_)))
					last_frame_time_ = bmd_time * AV_TIME_BASE / time_scale_;
				Core::AVSync sync(
					AVFrameFromDecklinkAudio(audioPacket, audio_channels_count_, AUDIO_SAMPLE_TYPE, BMDAudioSampleRate::bmdAudioSampleRate48kHz),
					AVFrameFromDecklinkVideo(videoFrame, current_field_order_, current_sar_, time_scale_),
					Core::FrameTimeInfo {
						TimeFromDeclinkTimecode(videoFrame, timecode_source_, current_format_.FrameRate()),
						last_frame_time_,
						AV_NOPTS_VALUE }
					);
				for (auto& provider : player_providers_)
					provider->Push(sync);
				for (auto& sink : output_sinks_)
					sink->Push(sync);
				if (frame_played_callback_)
					frame_played_callback_(sync.TimeInfo);
				return S_OK;
			}

			STDMETHODIMP						QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			STDMETHODIMP_(ULONG)				AddRef() override { return 1; }
			STDMETHODIMP_(ULONG)				Release() override { return 1; }

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

			void AddOutputSink(std::shared_ptr<Core::OutputSink>& output_sink)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				output_sinks_.push_back(output_sink);
			}

			void RemoveOutputSink(std::shared_ptr<Core::OutputSink> output_sink)
			{
				std::lock_guard<std::mutex> lock(channel_list_mutex_);
				auto item = std::find_if(output_sinks_.begin(), output_sinks_.end(), [&](const std::shared_ptr<Core::OutputSink>& p) { return p == output_sink; });
				if (item == output_sinks_.end())
					return;
				output_sinks_.erase(item);
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
				return current_field_order_;
			}

		};

		DecklinkInput::DecklinkInput(IDeckLink* decklink, Core::VideoFormatType initial_format, PixelFormat pixel_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video, bool format_autodetection)
			: impl_(std::make_unique<implementation>(decklink, initial_format, pixel_format, audio_channels_count, timecode_source, capture_video, format_autodetection))
		{ }
		
		DecklinkInput::~DecklinkInput()	{ }
		
		Core::AVSync DecklinkInput::PullSync(const Core::Player& player, int audio_samples_count) { return impl_->PullSync(player, audio_samples_count); }

		bool DecklinkInput::IsAddedToPlayer(const Core::Player& player) { return impl_->IsAddedToPlayer(player); }
		void DecklinkInput::AddToPlayer(const Core::Player& player) { impl_->AddToPlayer(player); }
		void DecklinkInput::RemoveFromPlayer(const Core::Player& player) { impl_->RemoveFromPlayer(player); }
		void DecklinkInput::AddOutputSink(std::shared_ptr<Core::OutputSink> output_sink) { impl_->AddOutputSink(output_sink); }
		void DecklinkInput::RemoveOutputSink(std::shared_ptr<Core::OutputSink> output_sink) { impl_->RemoveOutputSink(output_sink); }
		void DecklinkInput::Play() { }
		void DecklinkInput::Pause()	{ }
		bool DecklinkInput::IsPlaying() const { return true; }
		bool DecklinkInput::IsEof() const { return false; }
		int DecklinkInput::GetWidth() const { return impl_->GetWidth(); }
		int DecklinkInput::GetHeight() const { return impl_->GetHeight(); }
		TVPlayR::FieldOrder DecklinkInput::GetFieldOrder() { return impl_->GetFieldOrder(); }
		int DecklinkInput::GetAudioChannelCount() { return impl_->GetAudioChannelsCount(); }
		bool DecklinkInput::HaveAlphaChannel() const { return false; }
		void DecklinkInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
		void DecklinkInput::SetFormatChangedCallback(FORMAT_CALLBACK format_changed_callback) { impl_->format_changed_callback_ = format_changed_callback; }
	}
}