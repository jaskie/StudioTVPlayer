#include "../pch.h"
#include "DecklinkOutput.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"
#include "DecklinkUtils.h"
#include "DecklinkVideoFrame.h"
#include "../Core/Player.h" //ClockTarget here
#include "../Core/OverlayBase.h"
#include "../Core/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/SwResample.h"
#include "../DecklinkKeyerType.h"
#include "../TimecodeOutputSource.h"
#include "../Core/CoreUtils.h"
#include "../Common/Executor.h"

namespace TVPlayR {
	namespace Decklink {
		
		struct DecklinkOutput::implementation : IDeckLinkVideoOutputCallback, IDeckLinkNotificationCallback, Common::DebugTarget
		{
			const CComQIPtr<IDeckLinkOutput> output_;
			const CComQIPtr<IDeckLinkKeyer> decklink_keyer_;
			const CComQIPtr<IDeckLinkProfileAttributes> attributes_;
			const CComQIPtr<IDeckLinkNotification> notification_;
			const CComQIPtr<IDeckLinkStatus> status_;
			std::mutex preroll_mutex_;
			int index_;
			Core::VideoFormat format_;
			PixelFormat pixel_format_ = PixelFormat::yuv422;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			std::vector<std::shared_ptr<Core::ClockTarget>> clock_targets_;
			int preroll_buffer_size_ = 4;
			std::atomic_int64_t scheduled_frames_;
			std::atomic_int64_t  scheduled_samples_;
			int audio_channels_count_ = 0;
			Common::BlockingCollection<Core::AVSync> input_buffer_;
			Common::BlockingCollection<DecklinkVideoFrame*> decklink_frames_recycler_;
			std::shared_ptr<AVFrame> last_video_;
			std::atomic_int64_t last_video_time_;
			const DecklinkKeyerType keyer_;
			const TimecodeOutputSource timecode_source_;
			std::unique_ptr<FFmpeg::SwResample> audio_resampler_;
			Common::Executor overlay_executor_;
			

			implementation(IDeckLink *decklink, int index, DecklinkKeyerType keyer, TimecodeOutputSource timecode_source)
				: Common::DebugTarget(Common::DebugSeverity::info, "Decklink " + std::to_string(index))
				, output_(decklink)
				, attributes_(decklink)
				, decklink_keyer_(decklink)
				, notification_(decklink)
				, status_(decklink)
				, input_buffer_(2)
				, format_(Core::VideoFormatType::invalid)
				, index_(index)
				, keyer_(keyer)
				, timecode_source_(timecode_source)
				, overlay_executor_("Overlay queue for Decklink" + std::to_string(index), 3)
			{
				output_->SetScheduledFrameCompletionCallback(this);
			}

			~implementation()
			{
				Uninitialize();
				notification_->Unsubscribe(BMDNotifications::bmdStatusChanged, this);
				output_->SetScheduledFrameCompletionCallback(nullptr);
			}

			void OpenOutput(BMDDisplayMode mode, BMDPixelFormat pixel_format, int audio_channels_count)
			{
				if (!output_)
					THROW_EXCEPTION("Decklink Output: unable to find video output at index " + std::to_string(index_));
				BOOL modeSupported;
				BMDDisplayMode actualMode;
				if (FAILED(output_->DoesSupportVideoMode(BMDVideoConnection::bmdVideoConnectionUnspecified, mode, BMDPixelFormat::bmdFormat8BitYUV, BMDVideoOutputConversionMode::bmdNoVideoOutputConversion, BMDSupportedVideoModeFlags::bmdSupportedVideoModeDefault, &actualMode, &modeSupported))
					|| !modeSupported)
					THROW_EXCEPTION("Decklink Output at index " + std::to_string(index_) + ": unsupported video mode");
				if (pixel_format == BMDPixelFormat::bmdFormat8BitBGRA)
				{
					BOOL support = FALSE;
					switch (keyer_)
					{
					case DecklinkKeyerType::Internal:
						if (FAILED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsInternalKeying, &support)) || !support)
							break;
						decklink_keyer_->Enable(FALSE);
						decklink_keyer_->SetLevel(255);
						break;
					case DecklinkKeyerType::External:
						if (FAILED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsExternalKeying, &support)) || !support)
							break;
						decklink_keyer_->Enable(TRUE);
						decklink_keyer_->SetLevel(255);
						break;
					default:
						break;
					}
				}
				if (FAILED(output_->EnableVideoOutput(actualMode, static_cast<BMDVideoOutputFlags>(static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputRP188) | static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputVITC)))))
					THROW_EXCEPTION("Decklink Output at index " + std::to_string(index_) + ": unable to enable video output");
				if (keyer_ != DecklinkKeyerType::Internal)
					output_->EnableAudioOutput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, BMDAudioSampleType::bmdAudioSampleType32bitInteger, audio_channels_count, BMDAudioOutputStreamType::bmdAudioOutputStreamTimestamped);
				if (FAILED(notification_->Subscribe(BMDNotifications::bmdStatusChanged, this)))
					DebugPrintLine(Common::DebugSeverity::warning, "Failed to register notification callback.");
			}

			void Preroll()
			{
				if (!output_)
					return;
				std::lock_guard<std::mutex> lock(preroll_mutex_);
				scheduled_frames_ = 0LL;
				scheduled_samples_ = 0LL;
				output_->BeginAudioPreroll();
				for (size_t i = 0; i < preroll_buffer_size_; i++)
				{
					ScheduleAudio(FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32));
					ScheduleVideo(last_video_, last_video_time_);
				}
				output_->EndAudioPreroll();
			}

			void ScheduleVideo(const std::shared_ptr<AVFrame> &frame, std::int64_t timecode)
			{
				std::int64_t frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				DecklinkVideoFrame* decklink_frame;
				if (decklink_frames_recycler_.take(decklink_frame) != Common::BlockingCollectionStatus::Ok)
				{
					DebugPrintLine(Common::DebugSeverity::info, "ScheduleVideo: Can't take DecklinkVideoFrame from recycler");
					return;
				}
				decklink_frame->Update(format_, frame, timecode, frame_time);
				HRESULT ret = output_->ScheduleVideoFrame(decklink_frame, frame_time, format_.FrameRate().Denominator(), format_.FrameRate().Numerator());
				last_video_time_ = timecode;
				last_video_ = frame;
				scheduled_frames_++;
				if (FAILED(ret))
				{
					RecycleDecklinkFrame(decklink_frame);
					DebugPrintLine(Common::DebugSeverity::warning, "Unable to schedule DecklinkVideoFrame: " + std::to_string(decklink_frame->GetFrameTime()) + " HRESULT: " + std::to_string(ret));
				}
			}

			void RecycleDecklinkFrame(DecklinkVideoFrame *decklink_frame)
			{
				decklink_frame->Recycle();
				if (decklink_frames_recycler_.add(decklink_frame) != Common::BlockingCollectionStatus::Ok)
					decklink_frame->Release(); // if the frame is added to recycler, then decklink output will release it.
			}

			void ScheduleAudio(std::shared_ptr<AVFrame> &buffer)
			{
				if (!buffer)
					return;
				DebugPrintLineIf(buffer->nb_samples <= 0, Common::DebugSeverity::warning, "Empty audio frame");
				unsigned int samples_written = 0;
				if (keyer_ != DecklinkKeyerType::Internal)
					output_->ScheduleAudioSamples(buffer->data[0], buffer->nb_samples, scheduled_samples_, BMDAudioSampleRate::bmdAudioSampleRate48kHz, &samples_written);
				scheduled_samples_ += buffer->nb_samples;
				DebugPrintLineIf(keyer_ != DecklinkKeyerType::Internal && samples_written != buffer->nb_samples, Common::DebugSeverity::warning, "Not all samples written: " + std::to_string(samples_written) + " from buffer of " + std::to_string(buffer->nb_samples));
			}

			int AudioSamplesRequired() const
			{
				std::int64_t samples_required = av_rescale(scheduled_frames_ + 1, BMDAudioSampleRate::bmdAudioSampleRate48kHz * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - scheduled_samples_;
				if (samples_required < 0)
					samples_required = 0;
				return static_cast<int>(samples_required);
			}

			bool SetBufferSize(int size) 
			{
				if (format_.type() != Core::VideoFormatType::invalid)
					return false;
				preroll_buffer_size_ = size;
				return true;
			}

			void Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate)
			{
				if (video_format == Core::VideoFormatType::invalid)
					THROW_EXCEPTION("Decklink Output at index " + std::to_string(index_) + ": invalid video format");
				OpenOutput(GetDecklinkDisplayMode(video_format), BMDPixelFormatFromPixelFormat(pixel_format), audio_channel_count);
				format_ = video_format;
				pixel_format_ = pixel_format;
				audio_channels_count_ = audio_channel_count;
				audio_resampler_ = std::make_unique<FFmpeg::SwResample>(audio_channel_count, audio_sample_rate, AVSampleFormat::AV_SAMPLE_FMT_FLT, audio_channels_count_, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32);
				last_video_time_ = 0LL;
				last_video_ = FFmpeg::CreateEmptyVideoFrame(format_, pixel_format_);
				decklink_frames_recycler_.activate();
				for (size_t i = 0; i < preroll_buffer_size_ + 1; i++)
				{
					DecklinkVideoFrame* decklink_frame = new DecklinkVideoFrame(format_);
					decklink_frame->AddRef();
					RecycleDecklinkFrame(decklink_frame);
				}
				Preroll();
				output_->StartScheduledPlayback(0LL, format_.FrameRate().Numerator(), 1.0);
			}

			void Uninitialize()
			{
				if (format_.type() == Core::VideoFormatType::invalid)
					return;
				output_->StopScheduledPlayback(0LL, NULL, 0LL);
				format_ = Core::VideoFormatType::invalid;
				if (keyer_ != DecklinkKeyerType::Internal)
					output_->DisableAudioOutput();
				output_->DisableVideoOutput();
				DecklinkVideoFrame* decklink_frame;
				decklink_frames_recycler_.complete_adding();
				while (decklink_frames_recycler_.take(decklink_frame) == Common::BlockingCollectionStatus::Ok)
					decklink_frame->Release();
				audio_resampler_.reset();
			}

			void RegisterClockTarget(const std::shared_ptr<Core::ClockTarget> &target)
			{
				clock_targets_.push_back(target);
			}

			void UnregisterClockTarget(const std::shared_ptr<Core::ClockTarget> &target)
			{
				clock_targets_.erase(std::remove(clock_targets_.begin(), clock_targets_.end(), target), clock_targets_.end());
			}

			void AddOverlay(const std::shared_ptr<Core::OverlayBase> &overlay)
			{
				overlay_executor_.invoke([&]
					{
						overlays_.emplace_back(overlay);
					});
			}

			void RemoveOverlay(const std::shared_ptr<Core::OverlayBase> &overlay)
			{
				overlay_executor_.invoke([&]
					{
						overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
					});
			}

			void Push(const Core::AVSync &sync)
			{
				overlay_executor_.begin_invoke([sync, this]
					{
						Core::AVSync transformed(sync);
						for (auto &overlay : overlays_)
							transformed = overlay->Transform(transformed);
						if (input_buffer_.try_add(transformed) != Common::BlockingCollectionStatus::Ok)
							DebugPrintLine(Common::DebugSeverity::debug, "Frame dropped when pushed\n");
					});
			}

#pragma region IDeckLinkVideoOutputCallback
			HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame *completedFrame, BMDOutputFrameCompletionResult result) override
			{
				auto frame = dynamic_cast<DecklinkVideoFrame*>(completedFrame);
#ifdef DEBUG
				DebugPrintLine(result == BMDOutputFrameCompletionResult::bmdOutputFrameCompleted || result == BMDOutputFrameCompletionResult::bmdOutputFrameFlushed ? Common::DebugSeverity::trace : Common::DebugSeverity::info,
					"DecklinkVideoFrame " + std::to_string(frame->GetFrameTime()) + " " + BMDOutputFrameCompletionResultToString(result));
#endif
				RecycleDecklinkFrame(frame);

				if (result == BMDOutputFrameCompletionResult::bmdOutputFrameFlushed)
					return S_OK;
				std::lock_guard<std::mutex> lock(preroll_mutex_);
				int audio_samples_required = AudioSamplesRequired();
				for (auto &target : clock_targets_)
					target->RequestNextFrame(audio_samples_required);

				Core::AVSync sync;
				if (input_buffer_.try_take(sync) == Common::BlockingCollectionStatus::Ok)
				{
					ScheduleVideo(sync.Video, Core::TimecodeFromFameTimeInfo(sync.TimeInfo, timecode_source_));
					if (sync.Audio)
					{
						ScheduleAudio(audio_resampler_? audio_resampler_->Resample(sync.Audio) : sync.Audio);
					}
					else if (audio_samples_required > 0)
					{
						auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_required, audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32);
						DebugPrintLine(Common::DebugSeverity::trace, "Created empty audio with " + std::to_string(audio_samples_required) + " audio samples");
						ScheduleAudio(audio);
					}
				}
				else
				{
					DebugPrintLine(Common::DebugSeverity::debug, "AVSync not received from input buffer, replaying last video frame");
					ScheduleVideo(last_video_, last_video_time_);
					auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_required, audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32);
					ScheduleAudio(audio);
				}

				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped(void) override
			{
				return S_OK;
			}
#pragma endregion

#pragma region IDeckLinkNotificationCallback
			HRESULT STDMETHODCALLTYPE Notify(BMDNotifications topic, ULONGLONG param1, ULONGLONG param2)
			{
				if (topic == BMDNotifications::bmdStatusChanged && param1 == BMDDeckLinkStatusID::bmdDeckLinkStatusReferenceSignalLocked && keyer_ != DecklinkKeyerType::Internal)
				{
					BOOL locked;
					if (SUCCEEDED(status_->GetFlag(BMDDeckLinkStatusID::bmdDeckLinkStatusReferenceSignalLocked, &locked)) && locked)
					{
						DebugPrintLine(Common::DebugSeverity::info, "Reference signal locked, restarting playback");
						output_->StopScheduledPlayback(0LL, NULL, 0LL);
						Preroll();
						output_->StartScheduledPlayback(0LL, format_.FrameRate().Numerator(), 1.0);
					}
				}
				return S_OK;
			}
#pragma endregion

#pragma region IUnknown
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			ULONG STDMETHODCALLTYPE AddRef() override { return S_OK; }
			ULONG STDMETHODCALLTYPE Release() override { return S_OK; }
#pragma endregion

		};

		DecklinkOutput::DecklinkOutput(IDeckLink* decklink, int index, DecklinkKeyerType keyer, TimecodeOutputSource timecode_source)
			: impl_(std::make_unique<implementation>(decklink, index, keyer, timecode_source))
		{}

		DecklinkOutput::~DecklinkOutput() { }

		bool DecklinkOutput::SetBufferSize(int size) { return impl_->SetBufferSize(size); }
		int DecklinkOutput::GetBufferSize() const { return impl_->preroll_buffer_size_; }
		void DecklinkOutput::Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) { return impl_->Initialize(video_format, pixel_format, audio_channel_count, audio_sample_rate); }
		void DecklinkOutput::AddOverlay(const std::shared_ptr<Core::OverlayBase> &overlay)	{ impl_->AddOverlay(overlay); }
		void DecklinkOutput::RemoveOverlay(const std::shared_ptr<Core::OverlayBase> &overlay) { impl_->RemoveOverlay(overlay); }
		void DecklinkOutput::Push(const Core::AVSync &sync) { impl_->Push(sync); }
		void DecklinkOutput::RegisterClockTarget(const std::shared_ptr<Core::ClockTarget> &target) { impl_->RegisterClockTarget(target); }
		void DecklinkOutput::UnregisterClockTarget(const std::shared_ptr<Core::ClockTarget> &target) { impl_->UnregisterClockTarget(target); }
	}
}

