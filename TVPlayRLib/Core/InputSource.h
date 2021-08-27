#pragma once
#include "../FFMpeg/AVSync.h"

namespace TVPlayR {
	namespace Core {

class Channel;
class AudioChannelMapEntry;
class StreamInfo;

class InputSource
{
public:
	typedef void(*TIME_CALLBACK) (int64_t);
	typedef void(*STOPPED_CALLBACK) (void);
	typedef void(*LOADED_CALLBACK) (void);
	virtual FFmpeg::AVSync PullSync(int audio_samples_count) = 0;
	virtual bool IsAddedToChannel(const Channel& channel) = 0;
	virtual void AddToChannel(const Channel& channel) = 0;
	virtual void RemoveFromChannel(const Core::Channel& channel) = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual bool IsPlaying() const = 0;
	virtual int64_t GetVideoStart() const { return 0LL; }
	virtual int64_t GetVideoDuration() const { return 0LL; }
	virtual int64_t GetAudioDuration() const { return 0LL; }
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual AVFieldOrder GetFieldOrder() = 0;
	virtual int GetAudioChannelCount() = 0;
	virtual bool HaveAlphaChannel() const = 0;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) = 0;
};

}}