#include "../pch.h"
#include "PreviewScaler.h"
#include "../Core/Channel.h"

namespace TVPlayR {
	namespace Preview {

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

PreviewScaler::PreviewScaler(Core::Channel& channel, int width, int height)
	: VideoFilterBase(AV_PIX_FMT_BGRA)
	, channel_(channel)
	, filter_str_(GetFilterString(channel.Format(), width, height))
{
}

void PreviewScaler::Push(std::shared_ptr<AVFrame> frame)
{
	if (!IsInitialized())
		VideoFilterBase::CreateFilterChain(frame, av_inv_q(channel_.Format().FrameRate().av()), filter_str_);
	VideoFilterBase::Push(frame);
}


}}