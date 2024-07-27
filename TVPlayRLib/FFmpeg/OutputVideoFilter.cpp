#include "../pch.h"
#include "OutputVideoFilter.h"

namespace TVPlayR
{
	namespace FFmpeg
	{
		OutputVideoFilter::OutputVideoFilter(const std::string& filter_str, AVPixelFormat output_pix_fmt, const std::string &name)
			: VideoFilterBase(output_pix_fmt, name)
		{
			VideoFilterBase::SetFilter(filter_str);
		}
	}
}
