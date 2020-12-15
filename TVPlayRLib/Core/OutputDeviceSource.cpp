#include "../pch.h"
#include "OutputDeviceSource.h"
#include "InputSource.h"
#include "VideoFormat.h"


namespace TVPlayR {
	namespace Core {

OutputDeviceSource::OutputDeviceSource(std::shared_ptr<InputSource>& source, const VideoFormat& format, const PixelFormat pixel_format, const int audio_channels_count)
	: source_(source)
	, output_video_filter_(*source, format, pixel_format)
	, audio_channels_count_(audio_channels_count)
{
	source->AddToOutput(this);
}

OutputDeviceSource::~OutputDeviceSource()
{
	source_->RemoveFromOutput(this);
}

FFmpeg::AVFramePtr OutputDeviceSource::PullVideo()
{
	while (true)
	{
		if (output_video_filter_.IsEof())
			return last_video_;
		auto filtered = output_video_filter_.Pull();
		if (filtered)
		{
			last_video_ = filtered;
			return filtered;
		}
		else
		{
			auto decoded_frame = source_->PullVideo();
			if (decoded_frame)
				output_video_filter_.Push(decoded_frame);
			else
			{
				if (!output_video_filter_.IsEof() && !output_video_filter_.IsFlushed())
					output_video_filter_.Flush();
			}
		}
	}
}

FFmpeg::AVFramePtr OutputDeviceSource::PullAudio(int audio_samples_count)
{
	return source_->PullAudio(audio_samples_count);
}

void OutputDeviceSource::ResetFilter()
{
	output_video_filter_.Reset();
}


}}