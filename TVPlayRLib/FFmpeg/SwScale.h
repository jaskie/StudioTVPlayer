#pragma once
#include "../Common/NonCopyable.h"


namespace TVPlayR {
	namespace FFmpeg {
		class SwScale : Common::NonCopyable
		{
		public:
			SwScale(int src_width, int src_height, AVPixelFormat src_pixel_format, int dest_width, int dest_height, AVPixelFormat dest_pixel_format);
			std::shared_ptr<AVFrame> Scale(const std::shared_ptr<AVFrame>& in_frame);
			inline const AVPixelFormat GetSrcPixelFormat() const { return src_pixel_format_; }
			inline const int GetSrcWidth() const { return src_width_; }
			inline const int GetSrcHeight() const { return src_height_; }
			inline const AVPixelFormat GetDestPixelFormat() const { return dest_pixel_format_; }
			inline const int GetDestWidth() const { return dest_width_; }
			inline const int GetDestHeight() const { return dest_height_; }
		private:
			const AVPixelFormat src_pixel_format_;
			const int src_width_;
			const int src_height_;
			const AVPixelFormat dest_pixel_format_;
			const int dest_width_;
			const int dest_height_;
			std::unique_ptr<SwsContext, std::function<void(SwsContext*)>> sws_;
		};
}}
