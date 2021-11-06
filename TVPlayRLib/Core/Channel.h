#pragma once

#include "../Common/NonCopyable.h"
#include "VideoFormat.h"

namespace TVPlayR {
	enum class PixelFormat;

	namespace Core {
		class InputSource;
		class OutputDevice;
		class OverlayBase;

class Channel final : public Common::NonCopyable
{
public:
	typedef void(*AUDIO_VOLUME_CALLBACK) (std::vector<double>&);
	Channel(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate);
	~Channel();
	bool AddOutput(std::shared_ptr<OutputDevice> device);
	void RemoveOutput(std::shared_ptr<OutputDevice> device);
	void SetFrameClock(std::shared_ptr<OutputDevice> clock);
	void Load(std::shared_ptr<InputSource> source);
	void Preload(std::shared_ptr<InputSource> source);
	void AddOverlay(std::shared_ptr<OverlayBase> overlay);
	void Clear();
	const VideoFormat& Format() const;
	const TVPlayR::PixelFormat PixelFormat() const;
	const int AudioChannelsCount() const;
	const AVSampleFormat AudioSampleFormat() const { return AVSampleFormat::AV_SAMPLE_FMT_S32; }
	const int AudioSampleRate() const;
	void SetVolume(double volume);
	void SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback);
	const std::string& Name() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
	void RequestFrame(int audio_samples_count);
};

}}
