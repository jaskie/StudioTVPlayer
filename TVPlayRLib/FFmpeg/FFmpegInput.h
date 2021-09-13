#pragma once
#include "../Core/InputSource.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
		class AudioChannelMapEntry;
		class StreamInfo;
		enum class FieldOrder;
	}
		namespace FFmpeg {

class FFmpegInput: public Core::InputSource
{
public:
	FFmpegInput(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	~FFmpegInput();
	virtual FFmpeg::AVSync PullSync(const Core::Channel& channel, int audio_samples_count);
	virtual bool Seek(const int64_t time);
	virtual bool IsEof() const;
	virtual bool IsAddedToChannel(const Core::Channel& channel) override;
	virtual void AddToChannel(const Core::Channel& channel) override;
	virtual void RemoveFromChannel(const Core::Channel& channel) override;
	virtual void Play() override;
	virtual void Pause() override;
	virtual bool IsPlaying() const override;
	void SetIsLoop(bool is_loop);
	virtual AVRational GetTimeBase() const;
	virtual AVRational GetFrameRate() const;
	int64_t GetAudioDuration() const override;
	int64_t GetVideoStart() const override;
	int64_t GetVideoDuration() const override;
	virtual int GetWidth() const override;
	virtual int GetHeight() const override;
	virtual Core::FieldOrder GetFieldOrder() override;
	virtual int GetAudioChannelCount() override;
	virtual bool HaveAlphaChannel() const override;
	virtual int StreamCount() const;
	virtual Core::StreamInfo& GetStreamInfo(int index);
	virtual void SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map);
	virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback);
	virtual void SetLoadedCallback(LOADED_CALLBACK loaded_callback);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}