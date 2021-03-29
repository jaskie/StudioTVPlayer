#include "../pch.h"
#include "PreviewScaler.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace FFmpeg {
std::string PreviewScaler::GetFilterString(const Core::VideoFormat& channel_format, int width, int height)
{
	std::ostringstream filter_str;
	filter_str << "scale=w=" << width << ":h=" << height << ":in_color_matrix=";
	if (channel_format.type() == Core::VideoFormatType::pal || channel_format.type() == Core::VideoFormatType::pal_fha ||
		channel_format.type() == Core::VideoFormatType::ntsc || channel_format.type() == Core::VideoFormatType::ntsc_fha)
		filter_str << "bt601";
	else
		filter_str << "bt709";
	filter_str << ":sws_flags=neighbor";
	return filter_str.str();
}

PreviewScaler::PreviewScaler(const Core::VideoFormat& channel_format, int width, int height)
	: VideoFilterBase(AV_PIX_FMT_BGRA)
	, channel_frame_rate_(channel_format.FrameRate())
	, filter_str_(GetFilterString(channel_format, width, height))
{
}

void PreviewScaler::Push(std::shared_ptr<AVFrame> frame)
{
	if (!IsInitialized())
		VideoFilterBase::CreateFilterChain(frame, av_inv_q(channel_frame_rate_.av()), filter_str_);
	VideoFilterBase::Push(frame);
}


}}