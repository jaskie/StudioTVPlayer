#include "../pch.h"
#include "PreviewSink.h"
#include "../FFmpeg/SwScale.h"
#include "../FFmpeg/AVSync.h"

namespace TVPlayR {
	namespace Preview {

		struct PreviewSink::implementation
		{
			const int output_width_;
			const int output_height_;
			std::shared_ptr<FFmpeg::SwScale> preview_scaler_;
			FRAME_PLAYED_CALLBACK frame_played_callback_ = nullptr;
			Common::Executor executor_;

			implementation(int output_width, int output_height)
				: output_width_(output_width)
				, output_height_(output_height)
				, executor_("InputPreview", 1)
			{

			}

			void Push(std::shared_ptr<AVFrame>& video)
			{
				if (!video)
					return;
				executor_.begin_invoke([this, video]
				{
					if (!frame_played_callback_)
						return;
					if (!preview_scaler_ || preview_scaler_->GetSrcWidth() != video->width || preview_scaler_->GetSrcHeight() != video->height || preview_scaler_->GetSrcPixelFormat() != video->format)
						preview_scaler_ = std::make_unique<FFmpeg::SwScale>(video->width, video->height, static_cast<AVPixelFormat>(video->format), output_width_, output_height_, AV_PIX_FMT_BGRA);
					auto scaled = preview_scaler_->Scale(video);
					frame_played_callback_(scaled);
				});
			}

			void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
			{
				executor_.invoke([this, frame_played_callback] {
					frame_played_callback_ = frame_played_callback;
				});
			}

		};

		PreviewSink::PreviewSink(int output_width, int output_height)
			:impl_(std::make_unique<implementation>(output_width, output_height))
		{ }

		PreviewSink::~PreviewSink() {	}
		
		void PreviewSink::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			impl_->SetFramePlayedCallback(frame_played_callback);
		}
		
		void PreviewSink::Push(FFmpeg::AVSync& sync) { impl_->Push(sync.Video); }
	}
}