#pragma once

namespace TVPlayR {
	enum class PixelFormat;

	namespace Core {
		class InputSource;
		class OutputSink;
		class OutputDevice;
		class OverlayBase;
		class VideoFormat;
		enum class VideoFormatType;

class Player final : public Common::NonCopyable
{
public:
	typedef std::function<void(std::vector<float>&, float)> AUDIO_VOLUME_CALLBACK;
	Player(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate);
	~Player();
	void AddOutputSink(std::shared_ptr<OutputSink> device);
	void RemoveOutputSink(std::shared_ptr<OutputSink> device);
	void SetFrameClock(std::shared_ptr<OutputDevice> clock);
	void Load(std::shared_ptr<InputSource> source);
	void Preload(std::shared_ptr<InputSource> source);
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
