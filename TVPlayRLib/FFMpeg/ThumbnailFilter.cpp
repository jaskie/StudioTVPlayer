#include "../pch.h"
#include "ThumbnailFilter.h"
#include "Utils.h"


namespace TVPlayR {
	namespace FFmpeg {

ThumbnailFilter::ThumbnailFilter(int height, std::shared_ptr<AVFrame> frame)
	:VideoFilter(av_make_q(1, 1), AV_PIX_FMT_RGB24)
	, thumbnail_frame_(frame)
{
	std::ostringstream filter;
	if (frame->interlaced_frame)
		filter << "yadif,";
	if (frame->width == 720 && frame->height == 608)
	{
		filter << "crop=720:576:0:32,";
		if (frame->sample_aspect_ratio.num == 152 && frame->sample_aspect_ratio.den == 135) // IMX 4:3
			filter << "setsar=16/15,";
		else
			filter << "setsar=64/45,";
	}
	filter << "scale=trunc(" << height << "*dar):" << height << ",setsar=1/1";
	VideoFilter::SetFilter(frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), frame->sample_aspect_ratio, filter.str());
}

std::shared_ptr<AVFrame> ThumbnailFilter::Pull()
{
	while (thumbnail_frame_)
	{
		Push(thumbnail_frame_);
		auto frame = VideoFilter::Pull();
		if (frame)
			return frame;
	}
	return nullptr;
}

}}