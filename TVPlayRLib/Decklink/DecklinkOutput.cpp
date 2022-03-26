#include "../pch.h"
#include "DecklinkOutput.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"
#include "../Core/Player.h"
#include "DecklinkUtils.h"
#include "DecklinkVideoFrame.h"
#include "../Core/OverlayBase.h"
#include "../FFmpeg/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/SwResample.h"

namespace TVPlayR {
	namespace Decklink {
		
		struct DecklinkOutput::implementation : IDeckLinkVideoOutputCallback, Common::DebugTarget
		{
			const CComQIPtr<IDeckLinkOutput> output_;
			const CComQIPtr<IDeckLinkKeyer> keyer_;
			const CComQIPtr<IDeckLinkAttributes> attributes_;
			int index_;
			Core::VideoFormat format_;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			std::vector<Core::ClockTarget*> clock_targets_;
			std::mutex overlays_mutex_;
			int buffer_size_ = 4;
			std::atomic_int64_t scheduled_frames_;
			std::atomic_int64_t  scheduled_samples_;
			int audio_channels_count_ = 2;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			std::shared_ptr<AVFrame> last_video_;
			std::atomic_int64_t last_video_time_;
			const bool internal_keyer_;
			std::unique_ptr<FFmpeg::SwResample> audio_resampler_;

			implementation(IDeckLink* decklink, bool internal_keyer, int index)
				: Common::DebugTarget(Common::DebugSeverity::info, "Decklink " + std::to_string(index))
				, output_(decklink)
				, attributes_(decklink)
				, keyer_(decklink)
				, buffer_(2)
				, format_(Core::VideoFormatType::invalid)
				, index_(index)
				, internal_keyer_(internal_keyer)
			{
				output_->SetScheduledFrameCompletionCallback(this);
			}

			~implementation()
			{
				output_->SetScheduledFrameCompletionCallback(nullptr);
				ReleasePlayer();
			}

			bool OpenOutput(BMDDisplayMode mode, BMDPixelFormat pixel_format, int audio_channels_count)
			{
				if (!output_)
					return false;
				BMDDisplayModeSupport modeSupport;
				if (FAILED(output_->DoesSupportVideoMode(mode, BMDPixelFormat::bmdFormat8BitYUV, BMDVideoOutputFlags::bmdVideoOutputRP188, &modeSupport, NULL))
					|| modeSupport == bmdDisplayModeNotSupported)
					return false;
				if (pixel_format == BMDPixelFormat::bmdFormat8BitBGRA)
				{
					BOOL support = FALSE;
					if (internal_keyer_ && SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsInternalKeying, &support)) && support)
						keyer_->Enable(FALSE);
					else if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsExternalKeying, &support)) && support)
						keyer_->Enable(TRUE);
					if (support)
						keyer_->SetLevel(255);
				}
				if (FAILED(output_->EnableVideoOutput(mode, static_cast<BMDVideoOutputFlags>(static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputRP188) | static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputVITC)))))
					return false;
				if (!internal_keyer_)
					output_->EnableAudioOutput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, BMDAudioSampleType::bmdAudioSampleType32bitInteger, audio_channels_count, BMDAudioOutputStreamType::bmdAudioOutputStreamTimestamped);
				return true;
			}
						
			void Preroll(const Core::Player& player)
			{
				if (!output_)
					return;
				scheduled_frames_ = 0LL;
				scheduled_samples_ = 0LL;
				output_->BeginAudioPreroll();
				auto empty_video_frame = FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat());
				for (size_t i = 0; i < buffer_size_; i++)
				{
					ScheduleAudio(FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32));
					ScheduleVideo(empty_video_frame, 0LL);
				}
				output_->EndAudioPreroll();
			}

			void ScheduleVideo(const std::shared_ptr<AVFrame>& frame, std::int64_t timecode)
			{
				std::int64_t frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				CComPtr<DecklinkVideoFrame> decklink_frame(new DecklinkVideoFrame(format_, frame, timecode));
				HRESULT ret = output_->ScheduleVideoFrame(decklink_frame, frame_time, format_.FrameRate().Denominator(), format_.FrameRate().Numerator());
				last_video_time_ = timecode;
				last_video_ = frame;
				scheduled_frames_++;
				DebugPrintLineIf(FAILED(ret), Common::DebugSeverity::warning, "Unable to schedule frame: " + std::to_string(frame->pts) + " HRESULT: " + std::to_string(ret));
			}

			void ScheduleAudio(std::shared_ptr<AVFrame>& buffer)
			{
				if (!buffer)
					return;
				unsigned int samples_written = 0;
				if (!internal_keyer_)
					output_->ScheduleAudioSamples(buffer->data[0], buffer->nb_samples, scheduled_samples_, BMDAudioSampleRate::bmdAudioSampleRate48kHz, &samples_written);
				scheduled_samples_ += buffer->nb_samples;
				DebugPrintLineIf(!internal_keyer_ && samples_written != buffer->nb_samples, Common::DebugSeverity::warning, "Not all samples written: " + std::to_string(samples_written) + " from buffer of " + std::to_string(buffer->nb_samples));
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
				buffer_size_ = size;
				return true;
			}

			bool AssignToPlayer(const Core::Player& player)
			{
				if (!OpenOutput(GetDecklinkDisplayMode(player.Format().type()), BMDPixelFormatFromPixelFormat(player.PixelFormat()), player.AudioChannelsCount()))
					return false;
				if (player.Format().type() == Core::VideoFormatType::invalid)
					THROW_EXCEPTION("Invalid video format");
				format_ = player.Format();
				audio_channels_count_ = player.AudioChannelsCount();
				if (player.AudioSampleFormat() != AVSampleFormat::AV_SAMPLE_FMT_S32)
					audio_resampler_ = std::make_unique<FFmpeg::SwResample>(player.AudioChannelsCount(), player.AudioSampleRate(), player.AudioSampleFormat(), audio_channels_count_, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32);
				last_video_time_ = 0LL;
				Preroll(player);
				output_->StartScheduledPlayback(0LL, format_.FrameRate().Numerator(), 1.0);
				return true;
			}

			void ReleasePlayer()
			{
				if (format_.type() == Core::VideoFormatType::invalid)
					return;
				format_ = Core::VideoFormatType::invalid;
				BMDTimeValue frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				BMDTimeValue actual_stop;
				output_->StopScheduledPlayback(frame_time, &actual_stop, format_.FrameRate().Numerator());
				if (!internal_keyer_)
					output_->DisableAudioOutput();
				output_->DisableVideoOutput();
				audio_resampler_.reset();
			}

			void RegisterClockTarget(Core::ClockTarget* target)
			{

			}

			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				std::lock_guard<std::mutex> lock(overlays_mutex_);
				overlays_.emplace_back(overlay);
			}

			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				std::lock_guard<std::mutex> lock(overlays_mutex_);
				overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
			}

			void Push(FFmpeg::AVSync& sync)
			{
				if (buffer_.try_add(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine(Common::DebugSeverity::debug, "Frame dropped when pushed\n");
			}

			//IDeckLinkVideoOutputCallback
			HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override
			{
				if (result == BMDOutputFrameCompletionResult::bmdOutputFrameFlushed)
					return S_OK;

				int audio_samples_required = AudioSamplesRequired();
				for (Core::ClockTarget* target : clock_targets_)
					target->RequestFrame(audio_samples_required);

				FFmpeg::AVSync sync;
				if (buffer_.try_take(sync) == Common::BlockingCollectionStatus::Ok)
				{
					{
						std::lock_guard<std::mutex> lock(overlays_mutex_);
						for (auto& overlay : overlays_)
							sync = overlay->Transform(sync);
					}
					ScheduleVideo(sync.Video, sync.Timecode);
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
					DebugPrintLine(Common::DebugSeverity::trace, "Frame not received");
					ScheduleVideo(last_video_, last_video_time_);
					auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_required, audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32);
					ScheduleAudio(audio);
				}

#ifdef DEBUG
				auto frame = static_cast<DecklinkVideoFrame*>(completedFrame);
				if (result != BMDOutputFrameCompletionResult::bmdOutputFrameCompleted)
				{
					std::stringstream msg;
					msg << "Frame: " << scheduled_frames_ << ": " << ((result == BMDOutputFrameCompletionResult::bmdOutputFrameDisplayedLate) ? "late" : "dropped");
					DebugPrintLine(Common::DebugSeverity::info, msg.str());
				}
				else
				{
					//std::stringstream msg;
					//msg << "Frame: " << scheduled_frames_ << ": " << frame->GetPts();
					//DebugPrintLine(msg.str());
				}
#endif


				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped(void) override
			{
				return S_OK;
			}

			//IUnknown
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			ULONG STDMETHODCALLTYPE AddRef() override { return S_OK; }
			ULONG STDMETHODCALLTYPE Release() override { return S_OK; }

		};

		DecklinkOutput::DecklinkOutput(IDeckLink * decklink, bool internal_keyer, int index)
			: impl_(std::make_unique<implementation>(decklink, internal_keyer, index))
		{}

		DecklinkOutput::~DecklinkOutput() { }

		bool DecklinkOutput::SetBufferSize(int size) { return impl_->SetBufferSize(size); }
		int DecklinkOutput::GetBufferSize() const { return impl_->buffer_size_; }
		bool DecklinkOutput::AssignToPlayer(const Core::Player& player) { return impl_->AssignToPlayer(player); }
		void DecklinkOutput::ReleasePlayer()	{ impl_->ReleasePlayer(); }
		void DecklinkOutput::AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)	{ impl_->AddOverlay(overlay); }
		void DecklinkOutput::RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->RemoveOverlay(overlay); }
		void DecklinkOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void DecklinkOutput::RegisterClockTarget(Core::ClockTarget* target) { impl_->RegisterClockTarget(target); }
	}
}

