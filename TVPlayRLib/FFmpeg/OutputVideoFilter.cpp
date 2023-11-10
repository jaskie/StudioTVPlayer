#include "../pch.h"
#include "OutputVideoFilter.h"

namespace TVPlayR
{
	namespace FFmpeg
	{
		OutputVideoFilter::OutputVideoFilter(AVRational input_frame_rate, const std::string& filter_str, AVPixelFormat output_pix_fmt)
			: VideoFilterBase(output_pix_fmt)
		{
			VideoFilterBase::SetFilter(filter_str, av_inv_q(input_frame_rate));
		}

		bool OutputVideoFilter::Push(const std::shared_ptr<AVFrame>& frame)
		{
			return VideoFilterBase::Push(frame);
		}
	}
}
