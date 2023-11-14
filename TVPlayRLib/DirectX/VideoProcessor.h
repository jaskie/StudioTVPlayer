#pragma once
namespace TVPlayR
{
	namespace DirectX {

		class VideoProcessor
		{
		public:
			VideoProcessor();
			virtual ~VideoProcessor();
			bool Push(const std::shared_ptr<AVFrame>& frame);
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}
