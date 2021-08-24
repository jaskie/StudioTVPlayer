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
	virtual AVRational GetTimeBase() const = 0;
	virtual AVRational GetFrameRate() const = 0;
	virtual std::shared_ptr<AVFrame> GetFrameAt(int64_t time) = 0;
	virtual FFmpeg::AVSync PullSync(int audio_samples_count) = 0;
	virtual bool Seek(int64_t time) = 0;
	virtual bool IsAddedToChannel(Channel& channel) = 0;
	virtual void AddToChannel(Channel& channel) = 0;
	virtual void RemoveFromChannel() = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual bool IsPlaying() const = 0;
	virtual int StreamCount() const = 0;
	virtual StreamInfo& GetStreamInfo(int index) = 0;
	virtual void SetupAudio(const std::vector<AudioChannelMapEntry>& audio_channel_map) = 0;
	virtual int64_t GetVideoStart() const { return 0LL; }
	virtual int64_t GetVideoDuration() const { return 0LL; }
	virtual int64_t GetAudioDuration() const { return 0LL; }
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual AVFieldOrder GetFieldOrder() = 0;
	virtual int GetAudioChannelCount() = 0;
	virtual bool HaveAlphaChannel() const = 0;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) = 0;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback) = 0;
	virtual void SetLoadedCallback(LOADED_CALLBACK loaded_callback) = 0;
};

}}