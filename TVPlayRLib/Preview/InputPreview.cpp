#include "../pch.h"
#include "InputPreview.h"

namespace TVPlayR {
	namespace Preview {

		struct InputPreview::implementation
		{
			const int output_width_;
			const int output_height_;
			implementation(int output_width, int output_height)
				: output_width_(output_width)
				, output_height_(output_height)
			{

			}
		};

		InputPreview::InputPreview(int output_width, int output_height)
			:impl_(std::make_unique<implementation>(output_width, output_height))
		{ }

		InputPreview::~InputPreview() {	}
		
		void InputPreview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
		}
		
		void InputPreview::Push(std::shared_ptr<AVFrame> video)
		{
		}
	}
}