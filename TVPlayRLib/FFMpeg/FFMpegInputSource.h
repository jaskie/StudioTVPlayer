#pragma once
#include "Utils.h"
#include "../Core/InputSource.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace Core {
		class OutputDeviceSource;
	}
	namespace FFmpeg {

class FFmpegInputSource: public Core::InputSource
{
public:
	FFmpegInputSource(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount);
	~FFmpegInputSource();
	virtual FFmpeg::AVFramePtr PullVideo() override;
	virtual FFmpeg::AVFramePtr LastVideo() override;
	virtual FFmpeg::AVFramePtr PullAudio(int audio_samples_count) override;
	virtual bool Seek(const int64_t time) override;
	virtual bool IsEof() const;
	virtual AVRational GetTimeBase() const override;
	virtual AVRational GetFrameRate() const override;
	virtual void AddToOutput(Core::OutputDeviceSource* source) override;
	virtual void RemoveFromOutput(Core::OutputDeviceSource* source) override;
	virtual void SetupAudio(int channels) override;
	virtual int64_t GetVideoDuration() override;
	virtual int64_t GetAudioDuration() override;
	virtual int GetWidth() override;
	virtual int GetHeight() override;
	virtual AVFieldOrder GetFieldOrder() override;
	int64_t GetVideoDecoderTime();
	virtual void Play() override;
	virtual void Pause() override;
	virtual bool IsPlaying() const override;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}