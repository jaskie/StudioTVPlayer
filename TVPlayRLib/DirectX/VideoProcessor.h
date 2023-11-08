#pragma once
namespace TVPlayR
{
	namespace DirectX {

		class VideoProcessor
		{
		public:
			VideoProcessor();
			virtual ~VideoProcessor();
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}
