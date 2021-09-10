#include "../pch.h"
#include "ThumbnailFilter.h"
#include "FFmpegUtils.h"
#include "../Common/Rational.h"


namespace TVPlayR {
	namespace FFmpeg {

ThumbnailFilter::ThumbnailFilter(int width, int height, std::shared_ptr<AVFrame> frame)
	: VideoFilterBase(AV_PIX_FMT_RGB24)
	, input_frame_(frame)
	, width_(width)
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
	filter << "scale=" << width_ << ":" << height_ << ", setsar=1/1";
	VideoFilterBase::CreateFilterChain(input_frame_, time_base, filter.str());
	int frame_push_count = 3; // send max 3 frames to the filter
	while (!result_frame_ && --frame_push_count)
	{
		Push(input_frame_);
		result_frame_ = VideoFilterBase::Pull();
	}
	return result_frame_;
}

}}