#pragma once
#include "FilterBase.h"
#include "../Core/OverlayBase.h"

namespace TVPlayR {
	namespace Core {
		class VideoFormat;
	}
	namespace FFmpeg {
		
		class FFmpegInput;

		class VideoOverlayFilter final : public Core::OverlayBase
		{
		public:
			VideoOverlayFilter(const Core::VideoFormat& format, AVPixelFormat output_pixel_format);
			virtual ~VideoOverlayFilter();
			virtual Core::AVSync Transform(Core::AVSync& sync) override;
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}