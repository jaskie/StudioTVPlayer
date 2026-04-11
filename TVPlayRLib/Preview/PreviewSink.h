#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Core {
		struct AVSync;
	}
	namespace Preview {

		class PreviewSink final : public Core::OutputSink
		{
		public:
			explicit PreviewSink(int output_width, int output_height);
			virtual ~PreviewSink();
			typedef void(*FRAME_PLAYED_CALLBACK)(std::shared_ptr<AVFrame>);
			void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback);
			void Push(Core::AVSync& sync);
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}