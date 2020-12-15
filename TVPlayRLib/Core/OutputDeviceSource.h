#pragma once
#include "../FFMpeg/Utils.h"
#include "PixelFormat.h"
#include "../FFMpeg/OutputVideoFilter.h"

namespace TVPlayR {

	namespace Core {
		class InputSource;
		class VideoFormat;

class OutputDeviceSource
{
public:
	OutputDeviceSource(std::shared_ptr<InputSource>& source, const Core::VideoFormat& format, const PixelFormat pixel_format, const int audio_channels_count);
	~OutputDeviceSource();
	FFmpeg::AVFramePtr PullVideo();
	FFmpeg::AVFramePtr PullAudio(int samples_count);
	inline int AudioChannelsCount() const { return audio_channels_count_; }
	void ResetFilter();
private:
	std::shared_ptr<InputSource> source_;
	FFmpeg::OutputVideoFilter output_video_filter_;
	FFmpeg::AVFramePtr last_video_;
	const int audio_channels_count_;
};

}}

