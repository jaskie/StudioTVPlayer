#pragma once
#include "../Core/InputSource.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace Core {
		class AudioChannelMapEntry;
		class StreamInfo;
		enum class FieldOrder;
	}
		namespace FFmpeg {

class FFmpegInput: public Core::InputSource
{
public:
	FFmpegInput(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	virtual ~FFmpegInput();
	Core::AVSync PullSync(const Core::Player& player, int audio_samples_count);
	bool Seek(const std::int64_t time);
	bool IsEof() const;
	bool IsAddedToPlayer(const Core::Player& player) override;
	void AddToPlayer(const Core::Player& player) override;
	void RemoveFromPlayer(const Core::Player& player) override;
	void AddOutputSink(std::shared_ptr<Core::OutputSink> output_sink);
	void Play() override;
	void Pause() override;
	bool IsPlaying() const override;
	void SetIsLoop(bool is_loop);
	AVRational GetTimeBase() const;
	AVRational GetFrameRate() const;
	std::int64_t GetAudioDuration() const override;
	std::int64_t GetVideoStart() const override;
	std::int64_t GetVideoDuration() const override;
	int GetWidth() const override;
	int GetHeight() const override;
	TVPlayR::FieldOrder GetFieldOrder() override;
	int GetAudioChannelCount() override;
	bool HaveAlphaChannel() const override;
	virtual int StreamCount() const;
	const Core::StreamInfo& GetStreamInfo(int index) const;
	virtual void SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map);
	void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
	virtual void SetStoppedCallback(STOPPED_CALLBACK stopped_callback);
	virtual void SetLoadedCallback(LOADED_CALLBACK loaded_callback);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}