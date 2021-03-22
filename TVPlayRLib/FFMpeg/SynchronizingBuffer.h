#pragma once

#include "Utils.h"
#include "AVSync.h"

namespace TVPlayR {
	namespace Core 
	{
		enum class VideoFormatType;
	}
	namespace FFmpeg {

class SynchronizingBuffer
{
public:
	SynchronizingBuffer(const Core::VideoFormat& format, int audio_channel_count, AVSampleFormat audio_sample_format, bool is_playing, int64_t duration, int64_t initial_sync);
	~SynchronizingBuffer();
	void PushAudio(const std::shared_ptr<AVFrame>& frame);
	void PushVideo(const std::shared_ptr<AVFrame>& frame, const AVRational& time_base);
	AVSync PullSync(int audio_samples_count);
	bool Full() const;
	bool Ready();
	void SetIsPlaying(bool is_playing);
	void Seek(int64_t time);
	void SetSynchro(int64_t time);
	bool IsFlushed() const;
	bool IsEof();
	void Flush();
	const Core::VideoFormatType VideoFormat();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}