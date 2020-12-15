#include "../pch.h"
#include "OutputVideoFilter.h"
#include "../Core/InputSource.h"
#include "../Core/PixelFormat.h"

namespace TVPlayR {
	namespace FFmpeg {

static AVPixelFormat VideoFormatToPixelFormat(const Core::PixelFormat format)
{
	switch (format)
	{
	case Core::PixelFormat::bgra:
		return AV_PIX_FMT_BGRA;
	case Core::PixelFormat::yuv422:
		return AV_PIX_FMT_UYVY422;
	default:
		THROW_EXCEPTION("invalid pixel format")
	}
}

OutputVideoFilter::OutputVideoFilter(const Core::InputSource& source, const Core::VideoFormat& format, const Core::PixelFormat pixel_format)
	: VideoFilter(source.GetFrameRate(), source.GetTimeBase(), VideoFormatToPixelFormat(pixel_format))
	, output_format_(format)
	, pixel_format_(pixel_format)
	, input_frame_rate_(source.GetFrameRate())
{
}


bool OutputVideoFilter::Push(AVFramePtr frame)
{
	if (!IsInitialized())
		VideoFilter::SetFilter(frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), frame->sample_aspect_ratio, Setup(frame));
	return VideoFilter::Push(frame);
}

std::string OutputVideoFilter::Setup(AVFramePtr& frame)
{
	if (frame->width == output_format_.width() && frame->height == output_format_.height()
		&& static_cast<AVPixelFormat>(frame->format) == VideoFormatToPixelFormat(pixel_format_) 
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
	if (input_frame_rate_ == output_format_.FrameRate() * 2)
	{
		if (height != output_format_.height())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
		if (!frame->interlaced_frame && output_format_.interlaced())
			 filter << "interlace,";
		else if (frame->interlaced_frame && !output_format_.interlaced())
			filter << "fps=" << output_format_.FrameRate().numerator() << "/" << output_format_.FrameRate().denominator() << ",";
	}
	else 
		if (input_frame_rate_ != output_format_.FrameRate())
		filter << "fps=" << output_format_.FrameRate().numerator() << "/" << output_format_.FrameRate().denominator() << ",";
	if (frame->interlaced_frame && output_format_.field_mode() == Core::VideoFormat::FieldMode::progressive)
		filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
	if (frame->interlaced_frame && output_format_.field_mode() != Core::VideoFormat::FieldMode::progressive)
	{
		if (height > output_format_.height())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=0,";// interlace, ";
		if (height < output_format_.height())
			filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
	}
	filter << "format=" << static_cast<int>(VideoFilter::GetOutputPixelFormat());
	return filter.str();
}

	
}}