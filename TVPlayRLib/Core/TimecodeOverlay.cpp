#include "../pch.h"
#include <gdiplus.h>
#include "TimecodeOverlay.h"
#include "../FFmpeg/SwScale.h"
#include "VideoFormat.h"
#include "PixelFormat.h"


namespace TVPlayR {
	namespace Core {

		class GdiplusInitializer {
		private:
			Gdiplus::GdiplusStartupInput	gdiplus_startup_input_;
			ULONG_PTR						gdiplus_token_;
		public:
			GdiplusInitializer() { Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input_, NULL); }
			~GdiplusInitializer() { Gdiplus::GdiplusShutdown(gdiplus_token_); }
		};


		struct TimecodeOverlay::implementation
		{

			const GdiplusInitializer			gdiplus_initializer_;
			const VideoFormat					video_format_;
			const bool							no_video_;
			std::unique_ptr<FFmpeg::SwScale>	in_scaler_;
			std::unique_ptr<FFmpeg::SwScale>	out_scaler_;
			Gdiplus::SolidBrush					background_;
			Gdiplus::SolidBrush					foreground_;
			Gdiplus::Font						font_;
			const Gdiplus::PointF				timecode_position_;
			Gdiplus::StringFormat				timecode_format_;
			Gdiplus::Rect						background_rect_;

			implementation::implementation(const VideoFormatType video_format, bool no_video)
				: video_format_(video_format)
				, no_video_(no_video)
				, timecode_position_(static_cast<Gdiplus::REAL>(video_format_.width() / 2), static_cast<Gdiplus::REAL>(video_format_.height() * 900 / 1000))
				, background_rect_(video_format_.width() / 4, video_format_.height() * 165 / 200, video_format_.width() / 2, video_format_.height() / 7)
				, background_(Gdiplus::Color(150, 16, 16, 16))
				, foreground_(Gdiplus::Color(255, 232, 232, 232))
				, font_(L"Tahoma", static_cast<Gdiplus::REAL>(video_format_.height() / 10), Gdiplus::FontStyle::FontStyleBold)
			{
				timecode_format_.SetAlignment(Gdiplus::StringAlignmentCenter);
				timecode_format_.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			}

			FFmpeg::AVSync Transform(FFmpeg::AVSync& sync)
			{
				if (!sync.Video)
					return sync;
				std::shared_ptr<AVFrame>& input_frame = sync.Video;
				if (no_video_)
				{
					auto frame = FFmpeg::CreateEmptyVideoFrame(video_format_, Core::PixelFormat::bgra);
					frame->pts = input_frame->pts;
					Draw(frame, sync.Time);
					return FFmpeg::AVSync(sync.Audio, frame, sync.Time);
				}
				else
				{
					if (input_frame->format != AV_PIX_FMT_BGRA && (!in_scaler_ || in_scaler_->GetSrcWidth() != input_frame->width || in_scaler_->GetSrcHeight() != input_frame->height || in_scaler_->GetSrcPixelFormat() != input_frame->format))
					{
						in_scaler_ = std::make_unique<FFmpeg::SwScale>(input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format), input_frame->width, input_frame->height, AV_PIX_FMT_BGRA);
						out_scaler_ = std::make_unique<FFmpeg::SwScale>(video_format_.width(), video_format_.height(), AV_PIX_FMT_BGRA, input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format));
					}
					std::shared_ptr<AVFrame> rgba_frame = in_scaler_ ?  in_scaler_->Scale(input_frame) : input_frame;
					Draw(rgba_frame, sync.Time);
					// if incomming frame pixel format is AV_PIX_FMT_BGRA only copy the frame, otherwise convert it back to the format
					return FFmpeg::AVSync(sync.Audio, out_scaler_ ? out_scaler_->Scale(rgba_frame) : rgba_frame, sync.Time);
				}
			}

			void Draw(std::shared_ptr<AVFrame>& video, int64_t time)
			{
				Gdiplus::Bitmap bitmap(video->width, video->height, video->linesize[0], PixelFormat32bppARGB, video->data[0]);
				Gdiplus::Graphics graphics(&bitmap);
				graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeDefault);
				graphics.FillRectangle(&background_, background_rect_);
				std::string timecode = video_format_.FrameNumberToString(static_cast<int>(av_rescale(time, video_format_.FrameRate().Numerator(), video_format_.FrameRate().Denominator() * AV_TIME_BASE)));
				CA2W ca2w(timecode.c_str());
				graphics.DrawString(ca2w, -1, &font_, timecode_position_, &timecode_format_, &foreground_);
				Gdiplus::BitmapData bmpData;
				bitmap.LockBits(&background_rect_, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
				bitmap.UnlockBits(&bmpData);
			}

		};

		TimecodeOverlay::TimecodeOverlay(const VideoFormatType video_format, bool no_video)
			: impl_(std::make_unique<implementation>(video_format, no_video))
		{ }

		TimecodeOverlay::~TimecodeOverlay() { }

		FFmpeg::AVSync TimecodeOverlay::Transform(FFmpeg::AVSync& sync) { return impl_->Transform(sync); }
	}
}