#include "../pch.h"
#include "PlayerScaler.h"
#include "../Core/InputSource.h"
#include "../PixelFormat.h"
#include "../ColorSpace.h"
#include "../FieldOrder.h"
#include "../Core/Player.h"

namespace TVPlayR {
	namespace FFmpeg {

static std::string ColorSpaceToString(enum ColorSpace color_space) {
	switch (color_space)
	{
	case ColorSpace::bt601:
		return "bt601";
	case ColorSpace::bt709:
		return "bt709";
	case ColorSpace::bt2020:
		return "bt2020";
	default:
		THROW_EXCEPTION("Invalid color space");
	}
}

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
	int input_height = frame->height;
	int input_width = frame->width;
	bool input_interlaced = frame->interlaced_frame;
	if (input_width == 720 && frame->height == 608)
	{
		filter << "crop=720:576:0:32,";
		if (frame->sample_aspect_ratio.num == 152 && frame->sample_aspect_ratio.den == 135) // IMX 4:3
			filter << "setsar=16/15,";
		else
			filter << "setsar=64/45,";
		input_height = 576;
	}
	if (input_frame_rate == output_format_.FrameRate())
	{
		if (input_interlaced)
		{
			if (input_height < output_format_.height() && output_format_.field_order() != TVPlayR::FieldOrder::Progressive) // bwdif used only when upscaling
				filter << "bwdif,scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ",interlace,";
			else if ((input_height != output_format_.height() || input_width != output_format_.width()) && output_format_.field_order() != TVPlayR::FieldOrder::Progressive)
				filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":out_color_matrix=" << ColorSpaceToString(output_format_.ColorSpace()) << ":interl=1,";
			else if (output_format_.field_order() == TVPlayR::FieldOrder::Progressive)
				filter << "yadif,";
		}
		else // progressive input
			if ((input_height != output_format_.height() || input_width != output_format_.width()))
				filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":out_color_matrix=" << ColorSpaceToString(output_format_.ColorSpace()) << ",";
	}
	else if (input_frame_rate == output_format_.FrameRate() * 2)
	{
		if (input_interlaced)
			filter << "yadif,";
		if (input_height != output_format_.height() || input_width != output_format_.width())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":out_color_matrix=" << ColorSpaceToString(output_format_.ColorSpace()) << ",";
		if (output_format_.interlaced())
			filter << "interlace,";
		else
			filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
	}
	else if (input_frame_rate * 2 == output_format_.FrameRate())
	{
		if (input_interlaced)
			filter << "bwdif,";
		if (input_height != output_format_.height() || input_width != output_format_.width())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":out_color_matrix=" << ColorSpaceToString(output_format_.ColorSpace()) << ",";
		if (!input_interlaced)
			filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
	}
	else if (input_frame_rate != output_format_.FrameRate())
	{
		if (input_interlaced)
			filter << "bwdif,"; // this will make the interlaced content as fluent as possible
		if (input_height != output_format_.height() || input_width != output_format_.width())
			filter << "scale=w=" << output_format_.width() << ":h=" << output_format_.height() << ":out_color_matrix=" << ColorSpaceToString(output_format_.ColorSpace()) << ",";
		filter << "fps=" << output_format_.FrameRate().Numerator() << "/" << output_format_.FrameRate().Denominator() << ",";
	}
	filter << "format=" << static_cast<int>(output_pixel_format_);
	return filter.str();
}
	
}}