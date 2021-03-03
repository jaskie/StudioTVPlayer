#pragma once

#include "Utils.h"
#include "AVSync.h"

namespace TVPlayR {
	namespace Core 
	{
		class VideoFormat;
	}
	namespace FFmpeg {

class FrameSynchronizer
{
public:
	FrameSynchronizer(const Core::VideoFormat& format, Core::PixelFormat pix_fmt, int audio_channel_count, AVSampleFormat sample_format, int sample_rate, bool is_playing, int64_t initial_sync);
	~FrameSynchronizer();
	void PushAudio(const std::shared_ptr<AVFrame>& frame);
	void PushVideo(const std::shared_ptr<AVFrame>& frame);
	AVSync PullSync(int audio_samples_count);
	bool Ready() const;
	void SetIsPlaying(bool is_playing);
	void Seek(int64_t time);
	void SetTimebases(AVRational audio_time_base, AVRational video_time_base);
	void SetSynchro(int64_t time);
	bool IsFlushed() const;
	bool IsEof() const;
	void Flush();
	const Core::VideoFormat& Format() const;
	const Core::PixelFormat& PixelFormat() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}