#include "../pch.h"
#include "InputPreview.h"
#include "PreviewScaler.h"
#include "../Common/Executor.h"

namespace TVPlayR {
	namespace Preview {

		struct InputPreview::implementation
		{
			const int output_width_;
			const int output_height_;
			PreviewScaler preview_scaler_;
			Common::Executor executor_;
			FRAME_PLAYED_CALLBACK frame_played_callback_ = nullptr;
			implementation(int output_width, int output_height)
				: output_width_(output_width)
				, output_height_(output_height)
				, preview_scaler_(output_width, output_height)
				, executor_("InputPreview", 1)
			{

			}

			void Push(std::shared_ptr<AVFrame>& video)
			{
				if (!video)
					return;
				executor_.begin_invoke([this, video]
				{
					preview_scaler_.Push(video);
					while (true)
					{
						auto scaled = preview_scaler_.Pull();
						if (!scaled)
							break;
						if (frame_played_callback_)
							frame_played_callback_(scaled);
					}
				});
			}

		};

		InputPreview::InputPreview(int output_width, int output_height)
			:impl_(std::make_unique<implementation>(output_width, output_height))
		{ }

		InputPreview::~InputPreview() {	}
		
		void InputPreview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			impl_->frame_played_callback_ = frame_played_callback;
		}
		
		void InputPreview::Push(std::shared_ptr<AVFrame> video) { impl_->Push(video); }
	}
}