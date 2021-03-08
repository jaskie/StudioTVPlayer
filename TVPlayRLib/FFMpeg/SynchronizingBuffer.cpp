#include "../pch.h"
#include "SynchronizingBuffer.h"
#include "AudioFifo.h"
#include "../Core/VideoFormat.h"

//#undef DEBUG 

namespace TVPlayR {
	namespace FFmpeg {

#define SAMPLE_RATE 48000
#define SAMPLE_FORMAT AV_SAMPLE_FMT_S32

		struct SynchronizingBuffer::implementation {
			AVRational video_time_base_;
			AVRational audio_time_base_;
			const AVRational video_frame_rate_;
			const int sample_rate_;
			const int audio_channel_count_;
			const bool have_video_;
			const bool have_audio_;
			std::atomic_bool is_playing_;
			std::atomic_bool is_flushed_;
			int64_t sync_;
			std::mutex mutex_;
			const int64_t duration_;
			std::deque<std::shared_ptr<AVFrame>> video_queue_;
			std::unique_ptr<AudioFifo> fifo_;
			std::shared_ptr<AVFrame> last_video_;
			const Core::VideoFormatType video_format_;

			implementation(const Core::VideoFormat& format, int audio_channel_count, bool is_playing, int64_t duration, int64_t initial_sync)
				: video_format_(format.type())
				, video_time_base_(av_inv_q(format.FrameRate().av()))
				, video_frame_rate_(format.FrameRate().av())
				, sample_rate_(SAMPLE_RATE)
				, audio_time_base_(av_make_q(1, SAMPLE_RATE))
				, audio_channel_count_(audio_channel_count)
				, have_video_(true)
				, have_audio_(audio_channel_count)
				, is_playing_(is_playing)
				, duration_(duration)
				, sync_(initial_sync)
				, is_flushed_(false)
			{}

			void PushAudio(const std::shared_ptr<AVFrame>& frame)
			{
				if (!(frame && have_audio_))
					return;
				std::lock_guard<std::mutex> lock(mutex_);
				Sweep();
				if (!fifo_)
					fifo_ = std::make_unique<AudioFifo>(SAMPLE_FORMAT, audio_channel_count_, sample_rate_, audio_time_base_, PtsToTime(frame->pts, audio_time_base_), AV_TIME_BASE * 10);
				if (!fifo_->TryPush(frame))
				{
					fifo_->Reset(PtsToTime(frame->pts, audio_time_base_));
#ifdef DEBUG
					OutputDebugStringA("Audio fifo overflow. Flushing.\n");
#endif // DEBUG
					fifo_->TryPush(frame);
				}
			}

			void PushVideo(const std::shared_ptr<AVFrame>& frame)
			{
				if (!(frame && have_video_))
					return;
				std::lock_guard<std::mutex> lock(mutex_);
				Sweep();
				if (video_queue_.size() > video_frame_rate_.num * 10 / video_frame_rate_.den) // approx. 10 seconds
#ifdef DEBUG
				{
#endif // DEBUG
					video_queue_.clear();
#ifdef DEBUG
					OutputDebugStringA("Video queue overflow. Flushing.\n");
				}
#endif // DEBUG
				video_queue_.push_back(frame);
			}

			void Sweep()
			{
				int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, video_time_base_);
				int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, video_time_base_);
				int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
				int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
				if (min_audio == AV_NOPTS_VALUE)
					while (video_queue_.size() > 3)
						video_queue_.pop_front();
				if (fifo_ && min_video == AV_NOPTS_VALUE && (max_audio - min_audio) > 2 * duration_)
					fifo_->DiscardSamples(static_cast<int>(av_rescale(max_audio - min_audio - 2 * duration_, sample_rate_, AV_TIME_BASE)));
			}

			AVSync PullSync(int audio_samples_count)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				std::shared_ptr<AVFrame> audio;
				if (is_playing_)
				{
					int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, video_time_base_);
					int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
					int64_t time_delta = min_video == AV_NOPTS_VALUE || min_audio == AV_NOPTS_VALUE ? 0LL : min_video - min_audio + sync_;
#ifdef DEBUG
					//OutputDebugStringA(("Min audio:" + std::to_string(min_audio / 1000) + "\n").c_str());
					//OutputDebugStringA(("Min video:" + std::to_string(min_video / 1000) + "\n").c_str());
					//OutputDebugStringA(("Time delta:" + std::to_string(time_delta / 1000) + "\n").c_str());
#endif // DEBUG
					if (time_delta <= AV_TIME_BASE / 30)
					{
						if (!video_queue_.empty())
						{
							last_video_ = video_queue_.front();
							video_queue_.pop_front();
						}
					}
#ifdef DEBUG
					else
						OutputDebugStringA("Video frame repeated\n");
#endif // DEBUG

					if (time_delta >= AV_TIME_BASE / 20 && fifo_ && fifo_->SamplesCount() > 0)
					{
						int samples_to_discard = static_cast<int>(av_rescale(time_delta, sample_rate_, AV_TIME_BASE));
						fifo_->DiscardSamples(samples_to_discard);
					}
					if (fifo_)
						audio = fifo_->Pull(audio_samples_count);
				}
				if (!audio)
					audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channel_count_);
				if (!last_video_)
					last_video_ = video_queue_.front();
				return AVSync(audio, last_video_, PtsToTime(last_video_->pts, video_time_base_));
#ifdef DEBUG
				//if (audio->pts != AV_NOPTS_VALUE && last_video_->pts != AV_NOPTS_VALUE)
				//OutputDebugStringA(("Output video " + std::to_string(PtsToTime(last_video_->pts, video_time_base_)) + ", audio: " +  std::to_string(PtsToTime(audio->pts, audio_time_base_)) + ", delta:" + std::to_string((PtsToTime(last_video_->pts, video_time_base_) - PtsToTime(audio->pts, audio_time_base_)) / 1000) + "\n").c_str());
#endif // DEBUG
			}

			bool Ready() 
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if (is_playing_)
					if (is_flushed_)
						return !!last_video_;
					else
						return !video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() > av_rescale(sample_rate_, video_time_base_.num, video_time_base_.den));
				else
					return !!last_video_ || !video_queue_.empty();
			}

			bool Full() const
			{
				if (is_flushed_)
					return true;
				int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, video_time_base_);
				int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, video_time_base_);
				int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
				int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
				return max_video - min_video >= duration_ &&
					(!fifo_ || max_audio - min_audio >= duration_);
			}

			void SetIsPlaying(bool is_playing)
			{
				is_playing_ = is_playing;
			}

			void Seek(int64_t time)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if (fifo_)
					fifo_->Reset(time);
				video_queue_.clear();
				last_video_.reset();
				is_flushed_ = false;
			}

			void Flush()
			{
				is_flushed_ = true;
			}

			bool IsEof() 
			{
				std::lock_guard<std::mutex> lock(mutex_);
				return is_flushed_ && video_queue_.empty() && fifo_->SamplesCount() == 0;
			}

			void SetTimebases(AVRational audio_time_base, AVRational video_time_base)
			{
				audio_time_base_ = audio_time_base;
				video_time_base_ = video_time_base;
				is_flushed_ = false;
			}

			void SetSynchro(int64_t time)
			{
				sync_ = time;
#ifdef DEBUG
				OutputDebugStringA(("Sync set to " + std::to_string(time / 1000) + "\n").c_str());
#endif // DEBUG
			}

		};

	SynchronizingBuffer::SynchronizingBuffer(const Core::VideoFormat& format, int audio_channel_count, bool is_playing, int64_t duration, int64_t initial_sync)
		: impl_(std::make_unique<implementation>(format, audio_channel_count, is_playing, duration, initial_sync))
	{
	}

	SynchronizingBuffer::~SynchronizingBuffer() { }
	
	void SynchronizingBuffer::PushAudio(const std::shared_ptr<AVFrame>& frame) { impl_->PushAudio(frame); }
	void SynchronizingBuffer::PushVideo(const std::shared_ptr<AVFrame>& frame) { impl_->PushVideo(frame); }
	AVSync SynchronizingBuffer::PullSync(int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
	bool SynchronizingBuffer::Full() const { return impl_->Full(); }
	bool SynchronizingBuffer::Ready() { return impl_->Ready(); }
	void SynchronizingBuffer::SetIsPlaying(bool is_playing) { impl_->SetIsPlaying(is_playing); }
	void SynchronizingBuffer::Seek(int64_t time) { impl_->Seek(time); }
	void SynchronizingBuffer::SetTimebases(AVRational audio_time_base, AVRational video_time_base) { impl_->SetTimebases(audio_time_base, video_time_base); }
	void SynchronizingBuffer::SetSynchro(int64_t time) { impl_->SetSynchro(time); }
	bool SynchronizingBuffer::IsFlushed() const { return impl_->is_flushed_; }
	bool SynchronizingBuffer::IsEof() { return impl_->IsEof(); }
	void SynchronizingBuffer::Flush() { impl_->Flush(); }
	const Core::VideoFormatType SynchronizingBuffer::VideoFormat() { return impl_->video_format_; }
}}