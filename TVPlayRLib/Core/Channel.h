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
	Channel(const VideoFormatType& format, const Core::PixelFormat pixel_format, const int audio_channels_count);
	~Channel();
	bool AddOutput(std::shared_ptr<OutputDevice> device);
	void RemoveOutput(std::shared_ptr<OutputDevice> device);
	void SetFrameClock(std::shared_ptr<OutputDevice> clock);
	void Load(std::shared_ptr<InputSource> source);
	void Clear();
	const VideoFormat& Format() const;
	const PixelFormat PixelFormat() const;
	const int AudioChannelsCount() const;
	const AVSampleFormat AudioSampleFormat() const { return AVSampleFormat::AV_SAMPLE_FMT_S32; }
	void SetVolume(double volume);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
	void RequestFrame(int audio_samples_count);
};

}}
