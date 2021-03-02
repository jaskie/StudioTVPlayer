#include "../pch.h"
#include "FrameSynchronizer.h"
#include "AudioFifo.h"
#include "../Core/VideoFormat.h"

#undef DEBUG 

namespace TVPlayR {
	namespace FFmpeg {

		struct FrameSynchronizer::implementation {
			const Core::PixelFormat pix_fmt_;
			const Core::VideoFormat& format_;
			AVRational video_time_base_;
			AVRational audio_time_base_;
			const AVRational video_frame_rate_;
			const AVSampleFormat sample_format_;
			const int sample_rate_;
			const int audio_channel_count_;
			const bool have_video_;
			const bool have_audio_;
			bool is_playing_;
			bool is_flushed_;
			int64_t sync_;
			std::deque<std::shared_ptr<AVFrame>> video_queue_;
			std::unique_ptr<AudioFifo> fifo_;
			std::shared_ptr<AVFrame> empty_video_;
			std::shared_ptr<AVFrame> last_video_;

			implementation(const Core::VideoFormat& format, Core::PixelFormat pix_fmt, int audio_channel_count, AVSampleFormat sample_format, int sample_rate, bool is_playing, int64_t initial_sync)
				: format_(format)
				, pix_fmt_(pix_fmt)
				, video_time_base_(av_inv_q(format.FrameRate().av()))
				, video_frame_rate_(format.FrameRate().av())
				, sample_format_(sample_format)
				, sample_rate_(sample_rate)
				, audio_time_base_(av_make_q(1, sample_rate))
				, audio_channel_count_(audio_channel_count)
				, have_video_(true)
				, have_audio_(audio_channel_count)
				, is_playing_(is_playing)
				, sync_(initial_sync)
				, is_flushed_(false)
			{}

			void PushAudio(const std::shared_ptr<AVFrame>& frame)
			{
				if (!(frame && have_audio_))
					return;
				Sweep();
				if (!fifo_)
					fifo_.reset(new AudioFifo(sample_format_, audio_channel_count_, sample_rate_, audio_time_base_, PtsToTime(frame->pts, audio_time_base_), AV_TIME_BASE * 10));
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
				if (fifo_ && min_video == AV_NOPTS_VALUE && (max_audio - min_audio) > AV_TIME_BASE)
					fifo_->DiscardSamples(static_cast<int>(av_rescale(max_audio - min_audio - AV_TIME_BASE, sample_rate_, AV_TIME_BASE)));
			}

			AVSync PullSync(int audio_samples_count)
			{
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

				}
				else
					if (!last_video_)
						last_video_ = video_queue_.empty() ? EmptyVideo() : video_queue_.back();
					else if (last_video_ == empty_video_ && !video_queue_.empty())
						last_video_ = video_queue_.back();
				auto audio = PullAudio(audio_samples_count);
#ifdef DEBUG
				//if (audio->pts != AV_NOPTS_VALUE && last_video_->pts != AV_NOPTS_VALUE)
				//OutputDebugStringA(("Output video " + std::to_string(PtsToTime(last_video_->pts, video_time_base_)) + ", audio: " +  std::to_string(PtsToTime(audio->pts, audio_time_base_)) + ", delta:" + std::to_string((PtsToTime(last_video_->pts, video_time_base_) - PtsToTime(audio->pts, audio_time_base_)) / 1000) + "\n").c_str());
#endif // DEBUG
				if (!last_video_)
					last_video_ = EmptyVideo();
				return AVSync(audio, last_video_, PtsToTime(last_video_->pts, video_time_base_));
			}

			std::shared_ptr<AVFrame> PullAudio(int audio_samples_count)
			{
				std::shared_ptr<AVFrame> audio;
				if (is_playing_ && fifo_ && fifo_->SamplesCount() >= audio_samples_count)
					audio = fifo_->Pull(audio_samples_count);
				if (!audio)
				{
					audio = AllocFrame();
					audio->nb_samples = audio_samples_count;
					audio->channels = audio_channel_count_;
					audio->format = sample_format_;
					audio->channel_layout = AV_CH_LAYOUT_STEREO;
					av_frame_get_buffer(audio.get(), 0);
					av_samples_set_silence(audio->data, 0, audio_samples_count, audio_channel_count_, sample_format_);
#ifdef DEBUG
					OutputDebugStringA(("Got silent audio samples:" + std::to_string(audio->nb_samples) + "\n").c_str());
#endif // DEBUG
				}
				return audio;
			}

			std::shared_ptr<AVFrame> EmptyVideo()
			{
				if (!empty_video_)
					empty_video_ = CreateEmptyVideoFrame(format_, pix_fmt_);
#ifdef DEBUG
				OutputDebugStringA("Got empty video\n");
#endif // DEBUG
				return empty_video_;
			}

			bool Ready() const
			{
				return is_flushed_ ? true : !video_queue_.empty() && (!is_playing_ || (fifo_ && fifo_->SamplesCount() >= av_rescale(sample_rate_, video_frame_rate_.den, video_frame_rate_.num)));
			}

			void SetIsPlaying(bool is_playing)
			{
				is_playing_ = is_playing;
			}

			void Seek(int64_t time)
			{
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

			bool IsEof() const
			{
				return is_flushed_ && video_queue_.empty() && fifo_->SamplesCount() == 0;
			}

			void SetTimebases(AVRational audio_time_base, AVRational video_time_base)
			{
				audio_time_base_ = audio_time_base;
				video_time_base_ = video_time_base;
			}

			void SetSynchro(int64_t time)
			{
				sync_ = time;
#ifdef DEBUG
				OutputDebugStringA(("Sync set to " + std::to_string(time / 1000) + "\n").c_str());
#endif // DEBUG
			}

		};

	FrameSynchronizer::FrameSynchronizer(const Core::VideoFormat& format, Core::PixelFormat pix_fmt, int audio_channel_count, AVSampleFormat sample_format, int sample_rate, bool is_playing, int64_t initial_sync)
		: impl_(new implementation(format, pix_fmt, audio_channel_count, sample_format, sample_rate, is_playing, initial_sync))
	{
	}

	FrameSynchronizer::~FrameSynchronizer() { }
	
	void FrameSynchronizer::PushAudio(const std::shared_ptr<AVFrame>& frame) { impl_->PushAudio(frame); }
	void FrameSynchronizer::PushVideo(const std::shared_ptr<AVFrame>& frame) { impl_->PushVideo(frame); }
	AVSync FrameSynchronizer::PullSync(int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
	bool FrameSynchronizer::Ready() const { return impl_->Ready(); }
	void FrameSynchronizer::SetIsPlaying(bool is_playing) { impl_->SetIsPlaying(is_playing); }
	void FrameSynchronizer::Seek(int64_t time) { impl_->Seek(time); }
	void FrameSynchronizer::SetTimebases(AVRational audio_time_base, AVRational video_time_base) { impl_->SetTimebases(audio_time_base, video_time_base); }
	void FrameSynchronizer::SetSynchro(int64_t time) { impl_->SetSynchro(time); }
	bool FrameSynchronizer::IsFlushed() const { return impl_->is_flushed_; }
	bool FrameSynchronizer::IsEof() const { return impl_->IsEof(); }
	void FrameSynchronizer::Flush() { impl_->Flush(); }
}}