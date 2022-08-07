#pragma once
#include "PauseBuffer.h"

namespace TVPlayR {
	namespace Core 
	{
		class Player;
		struct AVSync;
		enum class VideoFormatType;
	}
	namespace FFmpeg {
		class AudioFifo;

class SynchronizingBuffer final : Common::NonCopyable, Common::DebugTarget
{
public:
	SynchronizingBuffer(const Core::Player * player, bool is_playing, std::int64_t duration, std::int64_t initial_sync, std::int64_t start_timecode, std::int64_t media_duration, FieldOrder field_order);
	~SynchronizingBuffer();
	void PushAudio(const std::shared_ptr<AVFrame>& frame);
	void PushVideo(const std::shared_ptr<AVFrame>& frame, const AVRational& time_base);
	Core::AVSync PullSync(int audio_samples_count);
	bool IsFull() const;
	bool IsReady() const;
	void SetIsPlaying(bool is_playing);
	void Seek(std::int64_t time);
	void Loop();
	void SetSynchro(std::int64_t time);
	bool IsFlushed() const;
	bool IsEof();
	void Flush();
	const Core::VideoFormatType VideoFormat() const;
private:
	const int sample_rate_;
	const int audio_channel_count_;
	const AVRational audio_time_base_;
	const AVRational video_frame_rate_;
	AVRational input_video_time_base_ = { 0, 1 };
	const bool have_video_;
	const bool have_audio_;
	std::atomic_bool is_playing_;
	std::atomic_bool is_flushed_;
	std::int64_t sync_;
	const size_t video_queue_size_;
	const int audio_fifo_size_;
	const std::int64_t capacity_;
	const std::int64_t start_timecode_;
	const std::int64_t media_duration_;
	std::deque<std::shared_ptr<AVFrame>> video_queue_;
	std::unique_ptr<AudioFifo> fifo_;
	std::unique_ptr<AudioFifo> fifo_loop_;
	PauseBuffer pause_frame_;
	const Core::VideoFormatType video_format_;
	const AVSampleFormat audio_sample_format_;
	void Sweep();
};

}}