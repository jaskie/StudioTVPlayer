#include "../pch.h"
#include "SynchronizingBuffer.h"
#include "../Core/AVSync.h"
#include "../Core/Player.h"
#include "../Core/VideoFormat.h"
#include "../FieldOrder.h"
#include "FFmpegUtils.h"
#include "AudioFifo.h"


namespace TVPlayR {
	namespace FFmpeg {

	SynchronizingBuffer::SynchronizingBuffer(AVRational video_frame_rate, AVRational video_time_base, AVRational audio_time_base, const Core::AudioParameters &audio_parameters, std::int64_t capacity, std::int64_t start_timecode, std::int64_t media_duration)
		: Common::DebugTarget(Common::DebugSeverity::info, "SynchronizingBuffer")
		, video_frame_rate_(video_frame_rate)
		, audio_parameters_(audio_parameters)
		, audio_time_base_(audio_time_base)
		, video_time_base_(video_time_base)
		, video_queue_size_(av_rescale(capacity, video_frame_rate_.num, video_frame_rate_.den * AV_TIME_BASE))
		, is_flushed_(false)
		, audio_fifo_size_(static_cast<int>(av_rescale(capacity, audio_parameters.SampleRate, AV_TIME_BASE)))
		, capacity_(capacity)
		, start_timecode_(start_timecode)
		, media_duration_(media_duration)
		, allow_push_(true)
		, allow_pull_(false)
	{
	}

	SynchronizingBuffer::~SynchronizingBuffer() { }
	
	void SynchronizingBuffer::PushAudio(const std::shared_ptr<AVFrame> &frame)
	{
		if (!(frame && audio_parameters_.ChannelCount))
			return;
		allow_push_.Wait();
		DebugPrintLine(Common::DebugSeverity::trace, "Push audio " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE));
		std::lock_guard<std::mutex> lock(content_mutex_);
		assert(!is_flushed_);
		if (!fifo_)
		{
			fifo_ = std::make_unique<AudioFifo>(audio_time_base_, audio_parameters_.SampleFormat, audio_parameters_.ChannelCount, audio_parameters_.SampleRate, FrameTime(frame), capacity_ * 4, "SynchronizingBuffer");
			DebugPrintLine(Common::DebugSeverity::info, "New fifo created");
		}
		if (!fifo_->Push(frame))
		{
			fifo_->Reset(FrameTime(frame));
			DebugPrintLine(Common::DebugSeverity::warning, "Audio fifo overflow. Flushing.");
			fifo_->Push(frame);
		}
		SetEvents();
	}

	void SynchronizingBuffer::PushVideo(const std::shared_ptr<AVFrame> &frame)
	{ 
		assert(!is_flushed_);
		allow_push_.Wait();
		DebugPrintLine(Common::DebugSeverity::trace, "Push video " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE));
		std::lock_guard<std::mutex> lock(content_mutex_);
		if (video_queue_.size() > video_frame_rate_.num * 10 / video_frame_rate_.den) // approx. 10 seconds
		{
			video_queue_.clear();
			DebugPrintLine(Common::DebugSeverity::warning, "Video queue overflow. Flushing.");
		}
		video_queue_.push_back(frame);
		SetEvents();
	}

	Core::AVSync SynchronizingBuffer::PullSync()
	{ 
		allow_pull_.Wait();
		std::lock_guard<std::mutex> lock(content_mutex_);
		assert(!video_queue_.empty());
		std::shared_ptr<AVFrame> video = video_queue_.front();
		video_queue_.pop_front();
		auto& fifo = (fifo_loop_) ? fifo_loop_ : fifo_;
		std::int64_t video_frame_start_time = FrameTime(video);
		std::int64_t video_frame_end_time = video_frame_start_time + av_rescale(AV_TIME_BASE, video_frame_rate_.den, video_frame_rate_.num);
		std::shared_ptr<AVFrame> audio = fifo
			? fifo->PullTimeRange(video_frame_start_time, video_frame_end_time)
			: nullptr;
		if (fifo_loop_ && fifo_loop_->SamplesCount() <= av_rescale(audio_parameters_.SampleRate / 2, video_time_base_.num, video_time_base_.den)) // less than half frame samples left
		{
			fifo_loop_.reset();
			DebugPrintLine(Common::DebugSeverity::info, "Loop fifo is destroyed");
		}
#ifdef DEBUG
		if (audio && audio->pts != AV_NOPTS_VALUE)
			DebugPrintLine(Common::DebugSeverity::trace, "PullSync: video " + std::to_string(static_cast<float>(FrameTime(video))/AV_TIME_BASE) + ", audio: " + std::to_string(static_cast<float>(PtsToTime(audio->pts, audio_time_base_))/AV_TIME_BASE) + ", delta:" + std::to_string((PtsToTime(video->pts, video_time_base_) - PtsToTime(audio->pts, audio_time_base_)) / 1000) + " ms");
#endif // DEBUG
		std::int64_t time = FrameTime(video);
		allow_push_.Set();
		if (!IsReady())
			allow_pull_.Reset();
		return Core::AVSync(audio, video, Core::FrameTimeInfo { time + start_timecode_, time, media_duration_ == AV_NOPTS_VALUE ? AV_NOPTS_VALUE : media_duration_ - time});
	}

	bool SynchronizingBuffer::IsFull() const 
	{ 
		if (is_flushed_)
			return true;
		int samples_count = fifo_ ? fifo_->SamplesCount() : INT_MAX;
		size_t video_queue_size = video_queue_.size();
		return video_queue_size >= video_queue_size_
			&& (samples_count > audio_fifo_size_);
	}

	bool SynchronizingBuffer::IsReady() const
	{ 
		if (is_flushed_)
			return true;
		return !video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() > av_rescale(audio_parameters_.SampleRate, video_time_base_.num, video_time_base_.den));
	}

	void SynchronizingBuffer::Seek(std::int64_t time)
	{ 
		std::lock_guard<std::mutex> lock(content_mutex_);
		if (fifo_)
			fifo_->Reset(time);
		video_queue_.clear();
		is_flushed_ = false;
		DebugPrintLine(Common::DebugSeverity::info, "Seek: " + std::to_string(time / 1000));
	}

	void SynchronizingBuffer::Loop()
	{
		if (!fifo_)
			return;
		std::lock_guard<std::mutex> lock(content_mutex_);
		int samples_over = static_cast<int>(fifo_->SamplesCount() - (video_queue_.size() * audio_parameters_.SampleRate * video_frame_rate_.den / video_frame_rate_.num));
		DebugPrintLine(Common::DebugSeverity::info, "Loop, samples over=" + std::to_string(samples_over));
		if (samples_over > 0)
			fifo_->DiscardSamples(samples_over);
		if (samples_over < 0)
			fifo_->Push(FFmpeg::CreateSilentAudioFrame(-samples_over, audio_parameters_.ChannelCount, audio_parameters_.SampleFormat));
		fifo_loop_ = std::move(fifo_);
		fifo_.reset();
	}
	
	bool SynchronizingBuffer::IsFlushed() const { return is_flushed_; }
	
	bool SynchronizingBuffer::IsEof() const { return is_flushed_ && video_queue_.empty() && (!fifo_ || fifo_->SamplesCount() == 0); }

	void SynchronizingBuffer::Flush()
	{
		is_flushed_ = true;
		DebugPrintLine(Common::DebugSeverity::info, "Set is_flushed");
	}

	void SynchronizingBuffer::Release()
	{
		allow_push_.Set();
		DebugPrintLine(Common::DebugSeverity::info, "Cleared");
	}

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
			fifo_->DiscardSamples(static_cast<int>(av_rescale(max_audio - min_audio - 2 * capacity_, audio_sample_rate_, AV_TIME_BASE)));*/
	}

	void SynchronizingBuffer::SetEvents()
	{
		if (IsFull())
			allow_push_.Reset();
		if (IsReady())
			allow_pull_.Set();
	}

}}