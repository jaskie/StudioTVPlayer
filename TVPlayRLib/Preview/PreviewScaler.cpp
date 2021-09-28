#include "../pch.h"
#include "PreviewScaler.h"
#include "../Core/Channel.h"

namespace TVPlayR {
	namespace Preview {

std::string PreviewScaler::GetFilterString(int output_width, int output_height)
{
	std::ostringstream filter_str;
	filter_str << "field,scale=w=" << output_width << ":h=" << output_height;
	/*if (pix_fmt == Core::PixelFormat::yuv422)
	{
		filter_str << ":in_color_matrix=";
		if (channel_format.type() == Core::VideoFormatType::pal || channel_format.type() == Core::VideoFormatType::pal_fha ||
			channel_format.type() == Core::VideoFormatType::ntsc || channel_format.type() == Core::VideoFormatType::ntsc_fha)
			filter_str << "bt601";
		else
			filter_str << "bt709";
	}*/
	return filter_str.str();
}

PreviewScaler::PreviewScaler(int output_width, int output_height)
	: VideoFilterBase(AV_PIX_FMT_BGRA)
	, filter_str_(GetFilterString(output_width, output_height))
{
}

void PreviewScaler::Push(std::shared_ptr<AVFrame> frame)
{
	if (!IsInitialized())
		VideoFilterBase::CreateFilterChain(frame, av_make_q(1, 1), filter_str_);
	VideoFilterBase::Push(frame);
}


}}