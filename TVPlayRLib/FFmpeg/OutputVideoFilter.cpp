#include "../pch.h"
#include "OutputVideoFilter.h"

namespace TVPlayR
{
	namespace FFmpeg
	{
		OutputVideoFilter::OutputVideoFilter(const std::string& filter_str, AVPixelFormat output_pix_fmt)
			: VideoFilterBase(output_pix_fmt)
		{
			VideoFilterBase::SetFilter(filter_str);
		}
	}
}
