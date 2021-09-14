#include "../pch.h"
#include "DecklinkOutput.h"
#include "../Common/ComInitializer.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"
#include "../Core/Channel.h"
#include "../Common/BlockingCollection.h"
#include "../Common/Executor.h"
#include "DecklinkUtils.h"
#include "DecklinkVideoFrame.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace Decklink {
		
		struct DecklinkOutput::implementation : IDeckLinkVideoOutputCallback, Common::DebugTarget
		{
			const CComQIPtr<IDeckLinkOutput> output_;
			const CComQIPtr<IDeckLinkKeyer> keyer_;
			const CComQIPtr<IDeckLinkAttributes> attributes_;
			int index_;
			Core::VideoFormat format_;
			int buffer_size_ = 4;
			std::atomic_int64_t scheduled_frames_;
			std::atomic_int64_t  scheduled_samples_;
			int audio_channels_count_ = 2;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			std::shared_ptr<AVFrame> last_video_;
			std::atomic_int64_t last_video_time_;

			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;

			implementation(IDeckLink* decklink, int index)
				: Common::DebugTarget(false, "Decklink " + std::to_string(index))
				, output_(decklink)
				, attributes_(decklink)
				, keyer_(decklink)
				, buffer_(2)
				, format_(Core::VideoFormatType::invalid)
				, index_(index)
			{
				output_->SetScheduledFrameCompletionCallback(this);
			}

			~implementation()
			{
				frame_requested_callback_ = nullptr;
				output_->SetScheduledFrameCompletionCallback(nullptr);
				ReleaseChannel();
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
					if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsExternalKeying, &support)) && support)
						keyer_->Enable(TRUE);
					else if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsInternalKeying, &support)) && support)
						keyer_->Enable(FALSE);
					if (support)
						keyer_->SetLevel(255);
				}
				if (FAILED(output_->EnableVideoOutput(mode, static_cast<BMDVideoOutputFlags>(static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputRP188) | static_cast<int>(BMDVideoOutputFlags::bmdVideoOutputVITC)))))
					return false;
				output_->EnableAudioOutput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, BMDAudioSampleType::bmdAudioSampleType32bitInteger, audio_channels_count, BMDAudioOutputStreamType::bmdAudioOutputStreamContinuous);
				return true;
			}
						
			void Preroll(Core::Channel& channel)
			{
				if (!output_)
					return;
				scheduled_frames_ = 0LL;
				scheduled_samples_ = 0LL;
				output_->BeginAudioPreroll();
				auto empty_video_frame = FFmpeg::CreateEmptyVideoFrame(channel.Format(), channel.PixelFormat());
				for (size_t i = 0; i < buffer_size_; i++)
				{
					ScheduleAudio(FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32));
					ScheduleVideo(empty_video_frame, 0LL);
					scheduled_frames_++;
				}
				output_->EndAudioPreroll();
			}

			bool ScheduleVideo(const std::shared_ptr<AVFrame>& frame, int64_t time)
			{
				int64_t frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				DecklinkVideoFrame* decklink_frame = new DecklinkVideoFrame(format_, frame, time);
				HRESULT ret = output_->ScheduleVideoFrame(decklink_frame, frame_time, format_.FrameRate().Denominator(), format_.FrameRate().Numerator());

				last_video_time_ = time;
				last_video_ = frame;
#ifdef DEBUG
				if (FAILED(ret))
				{
					std::stringstream msg;
					msg << "Unable to schedule frame: " << frame->pts;
					DebugPrintLine(msg.str());
				}
#endif			
				//if (FAILED(ret))
				//	decklink_frame->Release();
				return SUCCEEDED(ret);
			}

			void ScheduleAudio(const std::shared_ptr<AVFrame>& buffer)
			{
				unsigned int samples_written;
				output_->ScheduleAudioSamples(buffer->data[0], buffer->nb_samples, scheduled_samples_, BMDAudioSampleRate::bmdAudioSampleRate48kHz, &samples_written);
				scheduled_samples_ += buffer->nb_samples;
#ifdef DEBUG
				if (samples_written != buffer->nb_samples)
				{
					std::stringstream msg;
					msg << "Not all samples written: " << samples_written << " from buffer of " << buffer->nb_samples;
					DebugPrintLine(msg.str());
				}
#endif	
			}

			int AudioSamplesRequired() const
			{
				int64_t samples_required = av_rescale(scheduled_frames_ + 1, BMDAudioSampleRate::bmdAudioSampleRate48kHz * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - scheduled_samples_;
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

			bool AssignToChannel(Core::Channel& channel)
			{
				if (channel.AudioSampleFormat() != AVSampleFormat::AV_SAMPLE_FMT_S32)
					return false;
				if (!OpenOutput(GetDecklinkDisplayMode(channel.Format().type()), BMDPixelFormatFromVideoFormat(channel.PixelFormat()), channel.AudioChannelsCount()))
					return false;
				if (channel.Format().type() == Core::VideoFormatType::invalid)
					THROW_EXCEPTION("Invalid video format");
				format_ = channel.Format();
				audio_channels_count_ = channel.AudioChannelsCount();
				last_video_time_ = 0LL;
				Preroll(channel);
				output_->StartScheduledPlayback(0LL, format_.FrameRate().Numerator(), 1.0);
				return true;
			}

			void ReleaseChannel()
			{
				if (format_.type() == Core::VideoFormatType::invalid)
					return;
				format_ = Core::VideoFormatType::invalid;
				BMDTimeValue frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				BMDTimeValue actual_stop;
				output_->StopScheduledPlayback(frame_time, &actual_stop, format_.FrameRate().Numerator());
				output_->DisableAudioOutput();
				output_->DisableVideoOutput();
			}

			void Push(FFmpeg::AVSync& sync)
			{
				if (buffer_.try_add(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine("DecklinkOutput: Frame dropped when pushed\n");
			}

			//IDeckLinkVideoOutputCallback
			HRESULT __stdcall ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override
			{
				if (result == BMDOutputFrameCompletionResult::bmdOutputFrameFlushed)
					return S_OK;

				if (frame_requested_callback_)
					frame_requested_callback_(AudioSamplesRequired());

				FFmpeg::AVSync sync;
				if (buffer_.try_take(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine("DecklinkOutput: frame not received");

				if (!sync.Video)
					sync = FFmpeg::AVSync(FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_, AVSampleFormat::AV_SAMPLE_FMT_S32), last_video_, last_video_time_);

				if (ScheduleVideo(sync.Video, sync.Time))
					ScheduleAudio(sync.Audio);

#ifdef DEBUG
				auto frame = static_cast<DecklinkVideoFrame*>(completedFrame);
				if (result != BMDOutputFrameCompletionResult::bmdOutputFrameCompleted)
				{
					std::stringstream msg;
					msg << "Frame: " << scheduled_frames_ << ": " << ((result == BMDOutputFrameCompletionResult::bmdOutputFrameDisplayedLate) ? "late" : "dropped");
					DebugPrintLine(msg.str());
				}
				else
				{
					//std::stringstream msg;
					//msg << "Frame: " << scheduled_frames_ << ": " << frame->GetPts();
					//DebugPrintLine(msg.str());
				}
#endif

				scheduled_frames_++;
				return S_OK;
			}

			HRESULT __stdcall ScheduledPlaybackHasStopped(void) override
			{
				return S_OK;
			}

			//IUnknown
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			ULONG STDMETHODCALLTYPE AddRef() override { return S_OK; }
			ULONG STDMETHODCALLTYPE Release() override { return S_OK; }

		};

		DecklinkOutput::DecklinkOutput(IDeckLink * decklink, int index)
			: impl_(std::make_unique<implementation>(decklink, index))
		{}

		DecklinkOutput::~DecklinkOutput() { }

		bool DecklinkOutput::SetBufferSize(int size) { return impl_->SetBufferSize(size); }
		int DecklinkOutput::GetBufferSize() const { return impl_->buffer_size_; }
		bool DecklinkOutput::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }
		void DecklinkOutput::ReleaseChannel()	{ impl_->ReleaseChannel(); }
		void DecklinkOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void DecklinkOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->frame_requested_callback_ = frame_requested_callback; }
	}
}

