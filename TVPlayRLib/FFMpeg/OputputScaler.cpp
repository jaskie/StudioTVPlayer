#include "../pch.h"
#include "OputputScaler.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace FFmpeg {
std::string OputputScaler::GetFilterString(const Core::VideoFormat& channel_format, int width, int height)
{
	std::ostringstream filter_str;
	filter_str << "scale=" << width << ":" << height;
	return filter_str.str();
}

OputputScaler::OputputScaler(const Core::VideoFormat& channel_format, int width, int height)
	: VideoFilterBase(AV_PIX_FMT_BGRA)
	, channel_frame_rate_(channel_format.FrameRate())
	, filter_str_(GetFilterString(channel_format, width, height))
{
}

void OputputScaler::Push(std::shared_ptr<AVFrame> frame)
{
	if (!IsInitialized())
		VideoFilterBase::CreateFilterChain(frame, av_inv_q(channel_frame_rate_.av()), filter_str_);
	VideoFilterBase::Push(frame);
}


}}