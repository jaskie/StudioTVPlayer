#include "../pch.h"
#include "SynchronizingBuffer.h"
#include "../Core/AVSync.h"
#include "AudioFifo.h"
#include "../Core/Player.h"
#include "../Core/VideoFormat.h"
#include "FFmpegUtils.h"


namespace TVPlayR {
	namespace FFmpeg {

	SynchronizingBuffer::SynchronizingBuffer(const Core::Player * player, bool is_playing, std::int64_t capacity, std::int64_t initial_sync, std::int64_t start_timecode, std::int64_t media_duration, FieldOrder field_order)
		: Common::DebugTarget(Common::DebugSeverity::error, "SynchronizingBuffer " + player->Name())
		, video_format_(player->Format().type())
		, video_frame_rate_(player->Format().FrameRate().av())
		, sample_rate_(player->AudioSampleRate())
		, audio_time_base_(av_make_q(1, sample_rate_))
		, audio_channel_count_(player->AudioChannelsCount())
		, have_video_(true)
		, have_audio_(player->AudioChannelsCount() > 0)
		, is_playing_(is_playing)
		, video_queue_size_(av_rescale(capacity, video_frame_rate_.num, video_frame_rate_.den * AV_TIME_BASE))
		, sync_(initial_sync)
		, is_flushed_(false)
		, audio_sample_format_(player->AudioSampleFormat())
		, audio_fifo_size_(static_cast<int>(av_rescale(capacity, sample_rate_, AV_TIME_BASE)))
		, capacity_(capacity)
		, start_timecode_(start_timecode)
		, media_duration_(media_duration)
		, pause_buffer_(field_order, is_playing)
	{
		DebugPrintLine(Common::DebugSeverity::info, "Created");
	}

	SynchronizingBuffer::~SynchronizingBuffer() 
	{
		DebugPrintLine(Common::DebugSeverity::info, "Finalized");
	}
	
	void SynchronizingBuffer::PushAudio(const std::shared_ptr<AVFrame>& frame) 
	{
		if (!(frame && have_audio_))
			return;
		assert(!is_flushed_);
		DebugPrintLine(Common::DebugSeverity::trace, "Push audio " + std::to_string(static_cast<float>(PtsToTime(frame->pts, audio_time_base_)) / AV_TIME_BASE));
		Sweep();
		if (!fifo_)
		{
			fifo_ = std::make_unique<AudioFifo>(audio_sample_format_, audio_channel_count_, sample_rate_, audio_time_base_, PtsToTime(frame->pts, audio_time_base_), capacity_ * 4);
			DebugPrintLine(Common::DebugSeverity::info, "New fifo created");
		}
		if (!fifo_->TryPush(frame))
		{
			fifo_->Reset(PtsToTime(frame->pts, audio_time_base_));
			DebugPrintLine(Common::DebugSeverity::warning, "Audio fifo overflow. Flushing.");
			fifo_->TryPush(frame);
		}
	}

	void SynchronizingBuffer::PushVideo(const std::shared_ptr<AVFrame>& frame, const AVRational& time_base) 
	{ 
		if (!(frame && have_video_))
			return;
		input_video_time_base_ = time_base;
		DebugPrintLine(Common::DebugSeverity::trace, "Push video " + std::to_string(static_cast<float>(PtsToTime(frame->pts, input_video_time_base_)) / AV_TIME_BASE));
		assert(!is_flushed_);
		Sweep();
		if (video_queue_.size() > video_frame_rate_.num * 10 / video_frame_rate_.den) // approx. 10 seconds
		{
			video_queue_.clear();
			DebugPrintLine(Common::DebugSeverity::warning, "Video queue overflow. Flushing.");
		}
		video_queue_.push_back(frame);
	}
	
	Core::AVSync SynchronizingBuffer::PullSync(int audio_samples_count)
	{ 
		auto& fifo = (fifo_loop_) ? fifo_loop_ : fifo_;
		std::shared_ptr<AVFrame> audio = is_playing_ && fifo
			? fifo->Pull(audio_samples_count)
			: FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channel_count_, audio_sample_format_);
		if (fifo_loop_ && fifo_loop_->SamplesCount() <= av_rescale(sample_rate_ / 2, input_video_time_base_.num, input_video_time_base_.den)) // less than half frame samples left
		{
			fifo_loop_.reset();
			DebugPrintLine(Common::DebugSeverity::info, "Loop fifo is destroyed");
		}
		if ((is_playing_ || !pause_buffer_.GetFrame()) && !video_queue_.empty())
			pause_buffer_.SetFrame(video_queue_.front());
		if (is_playing_ && !video_queue_.empty())
			video_queue_.pop_front();
#ifdef DEBUG
		if (audio && audio->pts != AV_NOPTS_VALUE && !pause_buffer_.IsEmpty() && pause_buffer_.GetFrame()->pts != AV_NOPTS_VALUE)
			DebugPrintLine(Common::DebugSeverity::trace, "Output video " + std::to_string(static_cast<float>(PtsToTime(pause_buffer_.Pts(), input_video_time_base_))/AV_TIME_BASE) + ", audio: " + std::to_string(static_cast<float>(PtsToTime(audio->pts, audio_time_base_))/AV_TIME_BASE) + ", delta:" + std::to_string((PtsToTime(pause_buffer_.Pts(), input_video_time_base_) - PtsToTime(audio->pts, audio_time_base_)) / 1000) + " ms");
#endif // DEBUG
		std::int64_t time = PtsToTime(pause_buffer_.Pts(), input_video_time_base_);
		return Core::AVSync(audio, pause_buffer_.GetFrame(), Core::FrameTimeInfo{ time + start_timecode_, time, media_duration_ == AV_NOPTS_VALUE ? AV_NOPTS_VALUE : media_duration_ - time});
	}
	
	bool SynchronizingBuffer::IsFull() const 
	{ 
		if (is_flushed_)
			return true;
		return video_queue_.size() >= video_queue_size_
			&& (!fifo_ || fifo_->SamplesCount() > audio_fifo_size_);
		/*std::int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, video_time_base_);
		std::int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, video_time_base_);
		std::int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
		std::int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
		return max_video - min_video >= capacity_ &&
					(!fifo_ || max_audio - min_audio >= capacity_);*/
	}
	bool SynchronizingBuffer::IsReady()
	{ 
		if (is_flushed_)
			return true;
		if (is_playing_)
			return !video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() > av_rescale(sample_rate_, input_video_time_base_.num, input_video_time_base_.den));
		else
			return !pause_buffer_.IsEmpty() || !video_queue_.empty();
	}
	
	void SynchronizingBuffer::SetIsPlaying(bool is_playing) 
	{
		is_playing_ = is_playing;
		pause_buffer_.SetIsPlaying(is_playing);
		DebugPrintLine(Common::DebugSeverity::info, is_playing ? "Playing" : "Paused");
	}
	
	void SynchronizingBuffer::Seek(std::int64_t time) 
	{ 
		if (fifo_)
			fifo_->Reset(time);
		video_queue_.clear();
		pause_buffer_.Clear();
		is_flushed_ = false;
		DebugPrintLine(Common::DebugSeverity::info, "Seek: " + std::to_string(time / 1000));
	}

	void SynchronizingBuffer::Loop()
	{
		if (!fifo_)
			return;
		int samples_over = static_cast<int>(fifo_->SamplesCount() - (video_queue_.size() * sample_rate_ * video_frame_rate_.den / video_frame_rate_.num));
		DebugPrintLine(Common::DebugSeverity::info, "Loop, samples over=" + std::to_string(samples_over));
		if (samples_over > 0)
			fifo_->DiscardSamples(samples_over);
		if (samples_over < 0)
			fifo_->TryPush(FFmpeg::CreateSilentAudioFrame(-samples_over, audio_channel_count_, audio_sample_format_));
		fifo_loop_ = std::move(fifo_);
		fifo_.reset();
	}
	
	void SynchronizingBuffer::SetSynchro(std::int64_t time) 
	{ 
		sync_ = time;
		DebugPrintLine(Common::DebugSeverity::info, "Sync set to: " + std::to_string(time / 1000));
	}
	
	bool SynchronizingBuffer::IsFlushed() const { return is_flushed_; }
	
	bool SynchronizingBuffer::IsEof() { return is_flushed_ && video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() == 0); }
	
	void SynchronizingBuffer::Flush()
	{
		is_flushed_ = true;
		DebugPrintLine(Common::DebugSeverity::info, "Buffer flushed");
	}
	
	const Core::VideoFormatType SynchronizingBuffer::VideoFormat() const { return video_format_; }

	void SynchronizingBuffer::Sweep()
	{
		return;
		/*std::int64_t min_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.front()->pts, input_video_time_base_);
		std::int64_t max_video = video_queue_.empty() ? AV_NOPTS_VALUE : PtsToTime(video_queue_.back()->pts, input_video_time_base_);
		std::int64_t min_audio = fifo_ ? fifo_->TimeMin() : AV_NOPTS_VALUE;
		std::int64_t max_audio = fifo_ ? fifo_->TimeMax() : AV_NOPTS_VALUE;
		if (min_audio == AV_NOPTS_VALUE)
			while (video_queue_.size() > av_rescale(2 * capacity_, video_frame_rate_.num, video_frame_rate_.den * AV_TIME_BASE))
				video_queue_.pop_front();
		if (fifo_ && min_video == AV_NOPTS_VALUE && (max_audio - min_audio) > 2 * capacity_)
			fifo_->DiscardSamples(static_cast<int>(av_rescale(max_audio - min_audio - 2 * capacity_, sample_rate_, AV_TIME_BASE)));*/
	}
}}