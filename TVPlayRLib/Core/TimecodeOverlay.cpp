#include "../pch.h"
#include <gdiplus.h>
#include "TimecodeOverlay.h"
#include "../FFmpeg/SwScale.h"
#include "VideoFormat.h"
#include "../PixelFormat.h"


namespace TVPlayR {
	namespace Core {

		class GdiplusInitializer {
		private:
			Gdiplus::GdiplusStartupInput	gdiplus_startup_input_;
			ULONG_PTR						gdiplus_token_;
		public:
			GdiplusInitializer()  { Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input_, NULL); }
			~GdiplusInitializer() { Gdiplus::GdiplusShutdown(gdiplus_token_); }
		};


		struct TimecodeOverlay::implementation
		{

			const GdiplusInitializer			gdiplus_initializer_;
			const VideoFormat					video_format_;
			const TVPlayR::PixelFormat			output_pixel_format_;
			std::unique_ptr<FFmpeg::SwScale>	in_scaler_;
			std::unique_ptr<FFmpeg::SwScale>	out_scaler_;
			Gdiplus::SolidBrush					background_;
			Gdiplus::SolidBrush					foreground_;
			Gdiplus::Font						font_;
			Gdiplus::StringFormat				timecode_format_;
			float								scale_x_; // for formats with non-square pixels
			Gdiplus::Rect						background_rect_;
			Gdiplus::PointF						timecode_position_;


			implementation::implementation(const VideoFormatType video_format, TVPlayR::PixelFormat output_pixel_format)
				: video_format_(video_format)
				, output_pixel_format_(output_pixel_format)
				, background_rect_(GetBackgroundRect())
				, background_(Gdiplus::Color(150, 16, 16, 16))
				, foreground_(Gdiplus::Color(255, 232, 232, 232))
				, font_(L"Tahoma", static_cast<Gdiplus::REAL>(video_format_.height() / 9), Gdiplus::FontStyle::FontStyleBold)
				, timecode_position_(background_rect_.Width / (scale_x_ * 2), static_cast<Gdiplus::REAL>(background_rect_.Height * 21 / 40))
				, scale_x_(static_cast<float>(video_format_.SampleAspectRatio().Denominator()) / video_format_.SampleAspectRatio().Numerator())
			{
				timecode_format_.SetAlignment(Gdiplus::StringAlignmentCenter);
				timecode_format_.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			}


			Gdiplus::Rect GetBackgroundRect()
			{
				int height = video_format_.height() / 6;
				int width = static_cast<int>(video_format_.height() * scale_x_);
				return Gdiplus::Rect((video_format_.width() - width) / 2 , video_format_.height() - (height * 4 / 3) , width, height);
			}

			FFmpeg::AVSync Transform(FFmpeg::AVSync& sync)
			{
				if (!sync.Video)
					return sync;
				std::shared_ptr<AVFrame>& input_frame = sync.Video;
				assert(input_frame->width == video_format_.width() && input_frame->height == video_format_.height());
				if (!out_scaler_ && input_frame->format != AV_PIX_FMT_BGRA)
					out_scaler_ = std::make_unique<FFmpeg::SwScale>(video_format_.width(), video_format_.height(), AV_PIX_FMT_BGRA, input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format));
				if (!in_scaler_ && input_frame->format != AV_PIX_FMT_BGRA)
					in_scaler_ = std::make_unique<FFmpeg::SwScale>(input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format), input_frame->width, input_frame->height, AV_PIX_FMT_BGRA);
				std::shared_ptr<AVFrame> rgba_frame = in_scaler_ ? in_scaler_->Scale(input_frame) : input_frame;
				Draw(rgba_frame, sync.Timecode);
				// if incomming frame pixel format is AV_PIX_FMT_BGRA we draw directly on the frame
				return FFmpeg::AVSync(sync.Audio, out_scaler_ ? out_scaler_->Scale(rgba_frame) : rgba_frame, sync.Timecode);
			}

			void Draw(std::shared_ptr<AVFrame>& video, std::int64_t time)
			{
				Gdiplus::Bitmap frame_bitmap(video->width, video->height, video->linesize[0], PixelFormat32bppARGB, video->data[0]);
				Gdiplus::Graphics frame_graphics(&frame_bitmap);
				frame_graphics.FillRectangle(&background_, background_rect_);
				std::string timecode = time == AV_NOPTS_VALUE ?
					"NO TC DATA" :
					video_format_.FrameNumberToString(static_cast<int>(av_rescale(time, video_format_.FrameRate().Numerator(), video_format_.FrameRate().Denominator() * AV_TIME_BASE)));
				CA2W ca2w(timecode.c_str());
				Gdiplus::Bitmap overlay_bitmap(background_rect_.Width, background_rect_.Height, PixelFormat32bppARGB);
				Gdiplus::Graphics overlay_graphics(&overlay_bitmap);
				overlay_graphics.ScaleTransform(scale_x_, 1.0);
				overlay_graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
				overlay_graphics.DrawString(ca2w, -1, &font_, timecode_position_, &timecode_format_, &foreground_);
				frame_graphics.DrawImage(&overlay_bitmap, background_rect_);
				frame_graphics.Flush(Gdiplus::FlushIntentionSync);
			}
		};

		TimecodeOverlay::TimecodeOverlay(const VideoFormatType video_format, TVPlayR::PixelFormat output_pixel_format)
			: impl_(std::make_unique<implementation>(video_format, output_pixel_format))
		{ }

		TimecodeOverlay::~TimecodeOverlay() { }

		FFmpeg::AVSync TimecodeOverlay::Transform(FFmpeg::AVSync& sync) { return impl_->Transform(sync); }
	}
}