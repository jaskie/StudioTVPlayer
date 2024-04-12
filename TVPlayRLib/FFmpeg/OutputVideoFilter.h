#pragma once
#include "VideoFilterBase.h"

namespace TVPlayR {
	namespace Core {
		class Player;
	}
	namespace FFmpeg {
		/// <summary>
		/// Filter for sending video to FFMpegOutput
		/// </summary>
		class OutputVideoFilter :
			public VideoFilterBase
		{
		public:
			OutputVideoFilter(AVRational input_frame_rate, const std::string &filter_str, AVPixelFormat output_pix_fmt);
		};

	}
}
