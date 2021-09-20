#include "../pch.h"
#include <gdiplus.h>
#include "TimecodeOverlay.h"
#include "../FFmpeg/SwScale.h"


namespace TVPlayR {
	namespace Core {

		struct TimecodeOverlay::implementation
		{
			Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
			ULONG_PTR						gdiplusToken;
			std::unique_ptr<FFmpeg::SwScale> in_scaler_;
			std::unique_ptr<FFmpeg::SwScale> out_scaler_;

			implementation::implementation()
			{
				Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
			}

			implementation::~implementation()
			{
				Gdiplus::GdiplusShutdown(gdiplusToken);
			}


			FFmpeg::AVSync Transform(FFmpeg::AVSync& sync)
			{
				if (!sync.Video)
					return sync;
				std::shared_ptr<AVFrame>& video = sync.Video;
				if (!in_scaler_ || in_scaler_->GetSrcWidth() != video->width || in_scaler_->GetSrcHeight() != video->height || in_scaler_->GetSrcPixelFormat() != video->format)
				{
					in_scaler_ = std::make_unique<FFmpeg::SwScale>(video->width, video->height, static_cast<AVPixelFormat>(video->format), video->width, video->height, AV_PIX_FMT_ARGB);
					out_scaler_ = std::make_unique<FFmpeg::SwScale>(video->width, video->height, AV_PIX_FMT_ARGB, video->width, video->height, static_cast<AVPixelFormat>(video->format));
				}
				std::shared_ptr<AVFrame> argb_frame = in_scaler_->Scale(video);
				std::shared_ptr<AVFrame> result_frame = out_scaler_->Scale(argb_frame);
				return FFmpeg::AVSync(sync.Audio, result_frame, sync.Time);
			}
		};

		TimecodeOverlay::TimecodeOverlay()
			: impl_(std::make_unique<implementation>())
		{ }

		TimecodeOverlay::~TimecodeOverlay() { }

		FFmpeg::AVSync TimecodeOverlay::Transform(FFmpeg::AVSync& sync) { return impl_->Transform(sync); }
	}
}