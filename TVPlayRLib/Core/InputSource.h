#pragma once
#include "../FFMpeg/Utils.h"

namespace TVPlayR {
	namespace Core {

class VideoFormat;
class OutputDeviceSource;

class InputSource
{
public:
	typedef void(*TIME_CALLBACK) (int64_t);
	typedef void(*STOPPED_CALLBACK) (void);
	virtual AVRational GetTimeBase() const = 0;
	virtual AVRational GetFrameRate() const = 0;
	virtual FFmpeg::AVFramePtr PullVideo() = 0;
	virtual FFmpeg::AVFramePtr LastVideo() = 0;
	virtual FFmpeg::AVFramePtr PullAudio(int audio_samples_count) = 0;
	virtual bool Seek(int64_t time) = 0;
	virtual void AddToOutput(OutputDeviceSource* source) = 0;
	virtual void RemoveFromOutput(OutputDeviceSource* source) = 0;
	virtual void SetupAudio(int channels) = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual bool IsPlaying() const = 0;
	virtual int64_t GetVideoDuration() { return 0LL; }
	virtual int64_t GetAudioDuration() { return 0LL; }
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual AVFieldOrder GetFieldOrder() = 0;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) = 0;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback) = 0;
};

}}