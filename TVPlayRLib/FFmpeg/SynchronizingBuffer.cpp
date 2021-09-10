#include "../pch.h"
#include "SynchronizingBuffer.h"
#include "AudioFifo.h"
#include "../Core/Channel.h"
#include "../Common/Debug.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {


		struct SynchronizingBuffer::implementation : Common::DebugTarget<false>
		{
			const int sample_rate_;
			const int audio_channel_count_;
			const AVRational audio_time_base_;
			const AVRational video_frame_rate_;
			AVRational input_video_time_base_ = { 0, 1 };
			const bool have_video_;
			const bool have_audio_;
			std::atomic_bool is_playing_;
			std::atomic_bool is_flushed_;
			int64_t sync_;
			const int64_t duration_;
			std::deque<std::shared_ptr<AVFrame>> video_queue_;
			std::unique_ptr<AudioFifo> fifo_;
			std::shared_ptr<AVFrame> last_video_;
			const Core::VideoFormatType video_format_;
			const AVSampleFormat audio_sample_format_;

			implementation(const Core::Channel * channel, bool is_playing, int64_t duration, int64_t initial_sync)
				: video_format_(channel->Format().type())
				, video_frame_rate_(channel->Format().FrameRate().av())
				, sample_rate_(channel->AudioSampleRate())
				, audio_time_base_(av_make_q(1, sample_rate_))
				, audio_channel_count_(channel->AudioChannelsCount())
				, have_video_(true)
				, have_audio_(channel->AudioChannelsCount() > 0)
				, is_playing_(is_playing)
				, duration_(duration)
				, sync_(initial_sync)
				, is_flushed_(false)
				, audio_sample_format_(channel->AudioSampleFormat())
			{}

			void PushAudio(const std::shared_ptr<AVFrame>& frame)
			{
				if (!(frame && have_audio_))
					return;
				assert(!is_flushed_);
				Sweep();
				if (!fifo_)
					fifo_ = std::make_unique<AudioFifo>(audio_sample_format_, audio_channel_count_, sample_rate_, audio_time_base_, PtsToTime(frame->pts, audio_time_base_), AV_TIME_BASE * 10);
				if (!fifo_->TryPush(frame))
				{
					fifo_->Reset(PtsToTime(frame->pts, audio_time_base_));
					DebugPrintLine("Audio fifo overflow. Flushing.");
					fifo_->TryPush(frame);
				}
			}

			void PushVideo(const std::shared_ptr<AVFrame>& frame, const AVRational& time_base)
			{
				if (!(frame && have_video_))
					return;
				input_video_time_base_ = time_base;
				DebugPrintLine(("Push video " + std::to_string(PtsToTime(frame->pts, input_video_time_base_))));
				assert(!is_flushed_);
				Sweep();
				if (video_queue_.size() > video_frame_rate_.num * 10 / video_frame_rate_.den) // approx. 10 seconds
				{
					video_queue_.clear();
					DebugPrintLine("Video queue overflow. Flushing.");
				}
				video_queue_.push_back(frame);
			}

			void Sweep()
			{
				return;
				int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, input_video_time_base_);
				int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, input_video_time_base_);
				int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
				int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
				if (min_audio == AV_NOPTS_VALUE) 
					while (video_queue_.size() > av_rescale(2 * duration_, video_frame_rate_.num, video_frame_rate_.den * AV_TIME_BASE))
						video_queue_.pop_front();
				if (fifo_ && min_video == AV_NOPTS_VALUE && (max_audio - min_audio) > 2 * duration_)
					fifo_->DiscardSamples(static_cast<int>(av_rescale(max_audio - min_audio - 2 * duration_, sample_rate_, AV_TIME_BASE)));
			}

			AVSync PullSync(int audio_samples_count)
			{
				std::shared_ptr<AVFrame> audio = is_playing_ && fifo_ 
					? fifo_->Pull(audio_samples_count)
					: FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channel_count_, audio_sample_format_);
				if ((is_playing_ || !last_video_) && !video_queue_.empty())
					last_video_ = video_queue_.front();
				if (is_playing_ && !video_queue_.empty())
					video_queue_.pop_front();
#ifdef DEBUG
				if (audio->pts != AV_NOPTS_VALUE && last_video_->pts != AV_NOPTS_VALUE)
					DebugPrintLine(("Output video " + std::to_string(PtsToTime(last_video_->pts, input_video_time_base_)) + ", audio: " +  std::to_string(PtsToTime(audio->pts, audio_time_base_)) + ", delta:" + std::to_string((PtsToTime(last_video_->pts, input_video_time_base_) - PtsToTime(audio->pts, audio_time_base_)) / 1000)));
#endif // DEBUG
				return AVSync(audio, last_video_, PtsToTime(last_video_ ? last_video_->pts : AV_NOPTS_VALUE, input_video_time_base_));
			}

			bool Ready() const
			{
				if (is_flushed_)
					return true;
				if (is_playing_)
					return !video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() > av_rescale(sample_rate_, input_video_time_base_.num, input_video_time_base_.den));
				else
					return !!last_video_ || !video_queue_.empty();
			}

			bool Full() const
			{
				if (is_flushed_)
					return true;
				return video_queue_.size() >= av_rescale(duration_, video_frame_rate_.num, video_frame_rate_.den * AV_TIME_BASE)
					&& (!fifo_ || fifo_->SamplesCount() > av_rescale(duration_, sample_rate_, AV_TIME_BASE));
				/*int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, video_time_base_);
				int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, video_time_base_);
				int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
				int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
				return max_video - min_video >= duration_ &&
					(!fifo_ || max_audio - min_audio >= duration_);*/
			}

			void SetIsPlaying(bool is_playing)
			{
				is_playing_ = is_playing;
				DebugPrintLine(is_playing ? "Playing" : "Paused");
			}

			void Seek(int64_t time)
			{
				if (fifo_)
					fifo_->Reset(time);
				video_queue_.clear();
				last_video_.reset();
				is_flushed_ = false;
				DebugPrintLine(("Buffer seek: " + std::to_string(time/1000)));
			}

			void Flush()
			{
				is_flushed_ = true;
				DebugPrintLine("Buffer flushed");
			}

			bool IsEof() 
			{
				return is_flushed_ && video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() == 0);
			}

			void SetSynchro(int64_t time)
			{
				sync_ = time;
				DebugPrintLine(("Sync set to: " + std::to_string(time / 1000)));
			}

		};

	SynchronizingBuffer::SynchronizingBuffer(const Core::Channel * channel, bool is_playing, int64_t duration, int64_t initial_sync)
		: impl_(std::make_unique<implementation>(channel, is_playing, duration, initial_sync))
	{
	}

	SynchronizingBuffer::~SynchronizingBuffer() { }
	
	void SynchronizingBuffer::PushAudio(const std::shared_ptr<AVFrame>& frame) { impl_->PushAudio(frame); }
	void SynchronizingBuffer::PushVideo(const std::shared_ptr<AVFrame>& frame, const AVRational& time_base) { impl_->PushVideo(frame, time_base); }
	AVSync SynchronizingBuffer::PullSync(int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
	bool SynchronizingBuffer::Full() const { return impl_->Full(); }
	bool SynchronizingBuffer::Ready() const { return impl_->Ready(); }
	void SynchronizingBuffer::SetIsPlaying(bool is_playing) { impl_->SetIsPlaying(is_playing); }
	void SynchronizingBuffer::Seek(int64_t time) { impl_->Seek(time); }
	void SynchronizingBuffer::SetSynchro(int64_t time) { impl_->SetSynchro(time); }
	bool SynchronizingBuffer::IsFlushed() const { return impl_->is_flushed_; }
	bool SynchronizingBuffer::IsEof() { return impl_->IsEof(); }
	void SynchronizingBuffer::Flush() { impl_->Flush(); }
	const Core::VideoFormatType SynchronizingBuffer::VideoFormat() { return impl_->video_format_; }
}}