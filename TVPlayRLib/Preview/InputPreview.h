#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Preview {

		class InputPreview final : Common::NonCopyable
		{
		public:
			explicit InputPreview(int output_width, int output_height);
			~InputPreview();
			typedef void(*FRAME_PLAYED_CALLBACK)(std::shared_ptr<AVFrame>);
			void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback);
			void Push(std::shared_ptr<AVFrame> video);
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}