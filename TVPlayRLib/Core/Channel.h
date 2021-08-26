#pragma once

#include "../Common/NonCopyable.h"
#include "VideoFormat.h"
#include "../Core/PixelFormat.h"

namespace TVPlayR {
	namespace Core {
		class InputSource;
		class OutputDevice;

class Channel : public Common::NonCopyable
{
public:
	typedef void(*AUDIO_VOLUME_CALLBACK) (std::vector<double>&);
	Channel(const std::string& name, const VideoFormatType& format, const Core::PixelFormat pixel_format, const int audio_channels_count);
	~Channel();
	bool AddOutput(std::shared_ptr<OutputDevice> device);
	void RemoveOutput(std::shared_ptr<OutputDevice> device);
	void SetFrameClock(std::shared_ptr<OutputDevice> clock);
	void Load(std::shared_ptr<InputSource> source);
	void Preload(std::shared_ptr<InputSource> source);
	void Clear();
	const VideoFormat& Format() const;
	const PixelFormat PixelFormat() const;
	const int AudioChannelsCount() const;
	const AVSampleFormat AudioSampleFormat() const { return AVSampleFormat::AV_SAMPLE_FMT_S32; }
	const int AudioSampleRate() const { return 48000; }
	void SetVolume(double volume);
	void SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
	void RequestFrame(int audio_samples_count);
};

}}
