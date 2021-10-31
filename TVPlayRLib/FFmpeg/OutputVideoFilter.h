#pragma once
#include "VideoFilterBase.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {
		class OutputVideoFilter :
			public VideoFilterBase
		{
		public:
			OutputVideoFilter(const Core::Channel& channel, const std::string& filter_str, AVPixelFormat output_pix_fmt);

		};

	}
}
