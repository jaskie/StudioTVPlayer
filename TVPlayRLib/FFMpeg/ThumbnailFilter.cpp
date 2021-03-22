#include "../pch.h"
#include "ThumbnailFilter.h"
#include "Utils.h"


namespace TVPlayR {
	namespace FFmpeg {

ThumbnailFilter::ThumbnailFilter(int height, std::shared_ptr<AVFrame> frame)
	: VideoFilter(AV_PIX_FMT_RGB24)
	, input_frame_(frame)
	, height_(height)
{ }

std::shared_ptr<AVFrame> ThumbnailFilter::Pull()
{
	assert(input_frame_);
	if (result_frame_)
		return result_frame_;
	const AVRational time_base = av_make_q(1, 1);
	std::ostringstream filter;
	if (input_frame_->interlaced_frame)
		filter << "yadif,";
	if (input_frame_->width == 720 && input_frame_->height == 608)
	{
		filter << "crop=720:576:0:32,";
		if (input_frame_->sample_aspect_ratio.num == 152 && input_frame_->sample_aspect_ratio.den == 135) // IMX 4:3
			filter << "setsar=16/15,";
		else
			filter << "setsar=64/45,";
	}
	filter << "scale=trunc(" << height_ << "*dar):" << height_ << ",setsar=1/1";
	VideoFilter::SetFilter(input_frame_->width, input_frame_->height, static_cast<AVPixelFormat>(input_frame_->format), input_frame_->sample_aspect_ratio, time_base, filter.str());
	int frame_push_count = 3; // send max 3 frames to the filter
	while (!result_frame_ && --frame_push_count)
	{
		Push(input_frame_, time_base);
		result_frame_ = VideoFilter::Pull();
	}
	return result_frame_;
}

}}