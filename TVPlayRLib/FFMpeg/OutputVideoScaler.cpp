#include "../pch.h"
#include "OutputVideoScaler.h"
#include "../Core/InputSource.h"
#include "../Core/PixelFormat.h"

namespace TVPlayR {
	namespace FFmpeg {


OutputVideoScaler::OutputVideoScaler(AVRational input_frame_rate, AVRational input_time_base, const Core::VideoFormat& output_format, const AVPixelFormat output_pixel_format)
	: VideoFilter(input_time_base, output_pixel_format)
	, output_format_(output_format)
	, pixel_format_(output_pixel_format)
	, input_frame_rate_(input_frame_rate)
	, input_time_base_(input_time_base)
	, output_time_base_(av_inv_q(output_format.FrameRate().av()))
{
}


bool OutputVideoScaler::Push(std::shared_ptr<AVFrame> frame)
{
	if (!IsInitialized())
		VideoFilter::SetFilter(frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), frame->sample_aspect_ratio, Setup(frame));
	return VideoFilter::Push(frame);
}

std::string OutputVideoScaler::Setup(std::shared_ptr<AVFrame>& frame)
{
	if (frame->width == output_format_.width() && frame->height == output_format_.height()
		&& static_cast<AVPixelFormat>(frame->format) == pixel_format_
		&& output_format_.FrameRate() == input_frame_rate_)
		return "";

	std::ostringstream filter;
	int height = frame->height;
	if (frame->width == 720 && frame->height == 608)
	{
		filter << "crop=720:576:0:32,";
		if (frame->sample_aspect_ratio.num == 152 && frame->sample_aspect_ratio.den == 135) // IMX 4:3
			filter << "setsar=16/15,";
		else
			filter << "setsar=64/45,";
	}
	if (input_frame_rate_ == output_format_.FrameRate())
	{
		if (height != output_format_.height())
			if (frame->interlaced_frame && output_format_.field_mode() != Core::VideoFormat::FieldMode::progressive && height < output_format_.height()) // only when upscaling
				filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
			else
				filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=-1,";
	}
	else if (input_frame_rate_ == output_format_.FrameRate() * 2)
	{
		if (height != output_format_.height())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
		if (!frame->interlaced_frame && output_format_.interlaced())
			filter << "interlace,";
		else if (frame->interlaced_frame && !output_format_.interlaced())
			filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
	}
	else if (input_frame_rate_ != output_format_.FrameRate())
	{
		filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
		if (frame->interlaced_frame && output_format_.field_mode() == Core::VideoFormat::FieldMode::progressive)
			filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
		if (frame->interlaced_frame && output_format_.field_mode() != Core::VideoFormat::FieldMode::progressive)
		{
			if (height > output_format_.height())
				filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=0,";// interlace, ";
			if (height < output_format_.height())
				filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
		}
	}
	filter << "format=" << static_cast<int>(pixel_format_);
	return filter.str();
}

	
}}