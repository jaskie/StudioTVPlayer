#pragma once

namespace TVPlayR {
	enum class PixelFormat;

	namespace Core {
		class InputSource;
		class OutputSink;
		class FrameClockSource;
		class OutputDevice;
		class OverlayBase;
		class VideoFormat;
		enum class VideoFormatType;

class ClockTarget {
public:
	virtual void RequestFrame(int audio_samples_count) = 0;
	virtual ~ClockTarget() {}
};

class Player final : public ClockTarget, private Common::NonCopyable
{
public:
	typedef std::function<void(std::vector<float>&, float)> AUDIO_VOLUME_CALLBACK;
	Player(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate);
	virtual ~Player();
	void AddOutputSink(std::shared_ptr<OutputSink> device);
	void RemoveOutputSink(std::shared_ptr<OutputSink> device);
	void SetFrameClockSource(FrameClockSource& clock);
	void RequestFrame(int audio_samples_count) override;
	void Load(std::shared_ptr<InputSource> source);
	void PlayNext(std::shared_ptr<InputSource> source);
	void AddOverlay(std::shared_ptr<OverlayBase> overlay);
	void RemoveOverlay(std::shared_ptr<OverlayBase> overlay);
	void Clear();
	const VideoFormat& Format() const;
	const TVPlayR::PixelFormat PixelFormat() const;
	const int AudioChannelsCount() const;
	const AVSampleFormat AudioSampleFormat() const;
	const int AudioSampleRate() const;
	void SetVolume(float volume);
	void SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback);
	const std::string& Name() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
