#include "../pch.h"
#include "PlayerScaler.h"
#include "../Core/InputSource.h"
#include "../PixelFormat.h"
#include "../FieldOrder.h"
#include "../Core/Player.h"

namespace TVPlayR {
	namespace FFmpeg {


PlayerScaler::PlayerScaler(const Core::Player& player)
	: VideoFilterBase(TVPlayR::PixelFormatToFFmpegFormat(player.PixelFormat()))
	, output_format_(player.Format())
	, output_pixel_format_(TVPlayR::PixelFormatToFFmpegFormat(player.PixelFormat()))
{
}


bool PlayerScaler::Push(std::shared_ptr<AVFrame> frame, AVRational input_frame_rate, AVRational input_time_base)
{
	if (!IsInitialized())
		VideoFilterBase::SetFilter(GetFilterString(frame, input_frame_rate), input_time_base);
	return VideoFilterBase::Push(frame);
}

std::string PlayerScaler::GetFilterString(std::shared_ptr<AVFrame>& frame, Common::Rational<int> input_frame_rate)
{
	std::ostringstream filter;
	int height = frame->height;
	if (frame->width == 720 && frame->height == 608)
	{
		filter << "crop=720:576:0:32,";
		if (frame->sample_aspect_ratio.num == 152 && frame->sample_aspect_ratio.den == 135) // IMX 4:3
			filter << "setsar=16/15,";
		else
			filter << "setsar=64/45,";
		height = 576;
	}
	if (input_frame_rate == output_format_.FrameRate())
	{
		if (height != output_format_.height())
			if (frame->interlaced_frame && output_format_.field_order() != TVPlayR::FieldOrder::Progressive && height < output_format_.height()) // only when upscaling
				filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
			else
				filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=-1,";
	}
	else if (input_frame_rate == output_format_.FrameRate() * 2)
	{
		if (height != output_format_.height())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=-1,";
		if (!frame->interlaced_frame && output_format_.interlaced())
			filter << "interlace,";
		else if (frame->interlaced_frame && !output_format_.interlaced())
			filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
	}
	else if (input_frame_rate != output_format_.FrameRate())
	{
		filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
		if (frame->interlaced_frame)
		{
			if (output_format_.field_order() == TVPlayR::FieldOrder::Progressive)
				filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
			else
			{
				if (height > output_format_.height())
					filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":interl=0,";// interlace, ";
				if (height < output_format_.height())
					filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
			}
		} 
		else // input progressive
		{
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",";
		}
	}
	filter << "format=" << static_cast<int>(output_pixel_format_);
	return filter.str();
}

	
}}