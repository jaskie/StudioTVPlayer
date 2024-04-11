#pragma once
#include "../Core/AudioParameters.h"

namespace TVPlayR {
	
	enum class FieldOrder;
	
	namespace Core 
	{
		struct AVSync;
	}
	namespace FFmpeg {
		class AudioFifo;

class SynchronizingBuffer final : Common::NonCopyable, Common::DebugTarget
{
public:
	SynchronizingBuffer(AVRational video_frame_rate, AVRational video_time_base, AVRational audio_time_base, const Core::AudioParameters &audio_parameters, std::int64_t capacity, std::int64_t start_timecode, std::int64_t media_duration);
	virtual ~SynchronizingBuffer();
	void PushAudio(const std::shared_ptr<AVFrame> &frame);
	void PushVideo(const std::shared_ptr<AVFrame> &frame);
	Core::AVSync PullSync();
	bool IsFull() const;
	bool IsReady() const;
	void Seek(std::int64_t time);
	void Loop();
	bool IsFlushed() const;
	bool IsEof() const;
	void Flush();
private:
	const Core::AudioParameters audio_parameters_;
	const AVRational audio_time_base_;
	const AVRational video_frame_rate_;
	const AVRational video_time_base_;
	const bool have_video_;
	std::atomic_bool is_flushed_;
	const size_t video_queue_size_;
	const int audio_fifo_size_;
	const std::int64_t capacity_;
	const std::int64_t start_timecode_;
	const std::int64_t media_duration_;
	std::mutex content_mutex_;
	Common::ManualResetEvent push_event_;
	Common::ManualResetEvent pull_event_;
	std::deque<std::shared_ptr<AVFrame>> video_queue_;
	std::unique_ptr<AudioFifo> fifo_;
	std::unique_ptr<AudioFifo> fifo_loop_;
	void Sweep();
};

}}