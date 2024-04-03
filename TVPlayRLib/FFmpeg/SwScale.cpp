#include "../pch.h"
#include "SwScale.h"

namespace TVPlayR {
	namespace FFmpeg {

		SwScale::SwScale(int src_width, int src_height, AVPixelFormat src_pixel_format, int dest_width, int dest_height, AVPixelFormat dest_pixel_format)
			: src_width_(src_width)
			, src_height_(src_height)
			, src_pixel_format_(src_pixel_format)
			, dest_width_(dest_width)
			, dest_height_(dest_height)
			, dest_pixel_format_(dest_pixel_format)
			, sws_(sws_getContext(src_width, src_height, src_pixel_format, dest_width, dest_height, dest_pixel_format, 0, nullptr, nullptr, nullptr), 
				[](SwsContext* ctx) { sws_freeContext(ctx); })
		{ }

		std::shared_ptr<AVFrame> SwScale::Scale(const std::shared_ptr<AVFrame>& in_frame)
		{
			std::shared_ptr<AVFrame> out_frame = AllocFrame();
			out_frame->width = dest_width_;
			out_frame->height = dest_height_;
			out_frame->format = dest_pixel_format_;
			out_frame->pts = in_frame->pts;
			out_frame->interlaced_frame = in_frame->interlaced_frame;
			out_frame->top_field_first = in_frame->top_field_first;
			out_frame->sample_aspect_ratio = in_frame->sample_aspect_ratio;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(out_frame.get(), 0));
			if (sws_scale(sws_.get(), in_frame->data, in_frame->linesize, 0, in_frame->height, out_frame->data, out_frame->linesize) != dest_height_)
				THROW_EXCEPTION("SwScale: scale failed");
			return out_frame;
		}

}}