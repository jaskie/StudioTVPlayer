#pragma once
#include "Utils.h"
#include "../Core/InputSource.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
		class AudioChannelMapEntry;
		class StreamInfo;
	}
		namespace FFmpeg {

class FFmpegInputSource: public Core::InputSource
{
public:
	FFmpegInputSource(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount);
	~FFmpegInputSource();
	virtual std::shared_ptr<AVFrame> GetFrameAt(int64_t time) override;
	virtual FFmpeg::AVSync PullSync(int audio_samples_count);
	virtual bool Seek(const int64_t time) override;
	virtual bool IsEof() const;
	virtual bool IsAddedToChannel(Core::Channel& channel);
	virtual void AddToChannel(Core::Channel& channel) override;
	virtual void RemoveFromChannel() override;
	virtual void Play() override;
	virtual void Pause() override;
	virtual bool IsPlaying() const override;
	virtual AVRational GetTimeBase() const override;
	virtual AVRational GetFrameRate() const override;
	int64_t GetAudioDuration() const override;
	int64_t GetVideoStart() const override;
	int64_t GetVideoDuration() const override;
	virtual int GetWidth() override;
	virtual int GetHeight() override;
	virtual AVFieldOrder GetFieldOrder() override;
	virtual int GetAudioChannelCount() override;
	virtual int StreamCount() const override;
	virtual Core::StreamInfo& GetStreamInfo(int index) override;
	virtual void SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map) override;
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback) override;
	virtual void SetLoadedCallback(LOADED_CALLBACK loaded_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}