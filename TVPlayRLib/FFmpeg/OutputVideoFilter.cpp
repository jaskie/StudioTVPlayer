#include "../pch.h"
#include "OutputVideoFilter.h"
#include "../Core/Channel.h"
#include "../PixelFormat.h"

namespace TVPlayR
{
	namespace FFmpeg
	{
		OutputVideoFilter::OutputVideoFilter(const Core::Channel& channel, const std::string& filter_str, AVPixelFormat output_pix_fmt)
			: VideoFilterBase(output_pix_fmt)
		{
			const Core::VideoFormat& video_format = channel.Format();
			VideoFilterBase::CreateFilterChain(filter_str, video_format.width(), video_format.height(), PixelFormatToFFmpegFormat(channel.PixelFormat()), video_format.SampleAspectRatio().av(), video_format.FrameRate().invert().av());
		}
	}
}
