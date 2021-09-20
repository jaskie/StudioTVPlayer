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
			std::unique_ptr<Gdiplus::SolidBrush, std::function<void(Gdiplus::SolidBrush*)>> background_;
			std::unique_ptr<Gdiplus::SolidBrush, std::function<void(Gdiplus::SolidBrush*)>> foreground_;
			std::unique_ptr<Gdiplus::Font, std::function<void(Gdiplus::Font*)>> font_;

			implementation::implementation()
			{
				Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
				background_ = std::unique_ptr<Gdiplus::SolidBrush, std::function<void(Gdiplus::SolidBrush*)>>(new Gdiplus::SolidBrush(Gdiplus::Color(16, 16, 16, 180)), [](Gdiplus::SolidBrush* brush) { ::DeleteObject(brush); });
				foreground_ = std::unique_ptr<Gdiplus::SolidBrush, std::function<void(Gdiplus::SolidBrush*)>>(new Gdiplus::SolidBrush(Gdiplus::Color(232, 232, 232, 255)), [](Gdiplus::SolidBrush* brush) { ::DeleteObject(brush); });
			}

			implementation::~implementation()
			{
				Gdiplus::GdiplusShutdown(gdiplusToken);
			}


			FFmpeg::AVSync Transform(FFmpeg::AVSync& sync)
			{
				if (!sync.Video)
					return sync;
				std::shared_ptr<AVFrame>& input_frame = sync.Video;
				if (!in_scaler_ || in_scaler_->GetSrcWidth() != input_frame->width || in_scaler_->GetSrcHeight() != input_frame->height || in_scaler_->GetSrcPixelFormat() != input_frame->format)
				{
					in_scaler_ = std::make_unique<FFmpeg::SwScale>(input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format), input_frame->width, input_frame->height, AV_PIX_FMT_ARGB);
					out_scaler_ = std::make_unique<FFmpeg::SwScale>(input_frame->width, input_frame->height, AV_PIX_FMT_ARGB, input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format));
					font_ = std::unique_ptr<Gdiplus::Font, std::function<void(Gdiplus::Font*)>>(new Gdiplus::Font(L"Consolas", input_frame->height / 10), [](Gdiplus::Font* font) { ::DeleteObject(font); });
				}
				std::shared_ptr<AVFrame> argb_frame = in_scaler_->Scale(input_frame);
				Draw(argb_frame);
				return FFmpeg::AVSync(sync.Audio, out_scaler_->Scale(argb_frame), sync.Time);
			}

			void Draw(std::shared_ptr<AVFrame>& video)
			{
				Gdiplus::Bitmap bitmap(video->width, video->height, video->linesize[0], PixelFormat32bppARGB, video->data[0]);
				Gdiplus::Graphics graphics(&bitmap);
				Gdiplus::Rect r(video->width / 4, video->height * 7 / 10, video->width / 2, video->height / 5);
				Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255, 100), 60.f);
				graphics.FillRectangle(background_.get(), r);
				graphics.DrawString(L"Test", 4, font_.get(), Gdiplus::PointF(0, 0), foreground_.get());
				Gdiplus::BitmapData bmpData;
				Gdiplus::Rect all_image(0, 0, video->width, video->height);
				bitmap.LockBits(&all_image, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
				bitmap.UnlockBits(&bmpData);
			}

		};

		TimecodeOverlay::TimecodeOverlay()
			: impl_(std::make_unique<implementation>())
		{ }

		TimecodeOverlay::~TimecodeOverlay() { }

		FFmpeg::AVSync TimecodeOverlay::Transform(FFmpeg::AVSync& sync) { return impl_->Transform(sync); }
	}
}