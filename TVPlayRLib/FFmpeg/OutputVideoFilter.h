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
			OutputVideoFilter(AVRational input_frame_rate, const std::string& filter_str, AVPixelFormat output_pix_fmt);
			bool Push(std::shared_ptr<AVFrame> frame);
		};

	}
}
