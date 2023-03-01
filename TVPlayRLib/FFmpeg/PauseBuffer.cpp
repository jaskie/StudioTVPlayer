#include "../pch.h"
#include "PauseBuffer.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
		PauseBuffer::PauseBuffer(FieldOrder field_order, bool is_playing)
			: field_order_(field_order)
			, is_playing_(is_playing)
		{
		}

		void PauseBuffer::SetFrame(std::shared_ptr<AVFrame>& frame)
		{
			last_frame_ = frame;
		}

		std::shared_ptr<AVFrame> PauseBuffer::GetFrame()
		{
			return is_playing_ ? last_frame_ : GetStillFrame();
		}

		void PauseBuffer::SetIsPlaying(bool is_playing)
		{
			is_playing_ = is_playing;
			if (is_playing)
				still_frame_.reset();
		}
		
		bool PauseBuffer::IsEmpty() const
		{
			return !last_frame_;
		}

		int64_t PauseBuffer::Pts() const
		{
			return last_frame_ ? last_frame_->pts : AV_NOPTS_VALUE;
		}
		
		void PauseBuffer::Clear()
		{
			last_frame_.reset();
			still_frame_.reset();
		}

		std::shared_ptr<AVFrame>& PauseBuffer::GetStillFrame()
		{
			if (last_frame_ && !still_frame_)
			{
				switch (field_order_)
				{
				case FieldOrder::TopFieldFirst:
					still_frame_ = FrameToField(last_frame_, true);
					break;
				case FieldOrder::BottomFieldFirst:
					still_frame_ = FrameToField(last_frame_, false);
					break;
				default:
					still_frame_ = last_frame_;
					break;
				}
			}
			return still_frame_;
		}

		std::shared_ptr<AVFrame> PauseBuffer::FrameToField(std::shared_ptr<AVFrame>& source, bool top_field)
		{
			assert(field_order_ == FieldOrder::BottomFieldFirst || field_order_ == FieldOrder::TopFieldFirst);
			std::shared_ptr<AVFrame> dest = AllocFrame();
			dest->width = source->width;
			dest->height = source->height;
			dest->format = source->format;
			dest->pict_type = source->pict_type;
			dest->sample_aspect_ratio = source->sample_aspect_ratio;
			dest->interlaced_frame = source->interlaced_frame;
			dest->top_field_first = source->top_field_first;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(dest.get(), 0));

			const uint8_t* source_data[AV_NUM_DATA_POINTERS] = { 0 };
			int source_linesizes[AV_NUM_DATA_POINTERS] = { 0 };
			uint8_t* dest_data[AV_NUM_DATA_POINTERS] = { 0 };
			int dest_linesizes[AV_NUM_DATA_POINTERS] = { 0 };
			for (int plane = 0; plane < AV_NUM_DATA_POINTERS; plane++)
			{
				source_data[plane] = source->data[plane] + (top_field ? 0 : source->linesize[plane]);
				source_linesizes[plane] = source->linesize[plane] * 2;
				dest_data[plane] = dest->data[plane];
				dest_linesizes[plane] = dest->linesize[plane] * 2;
			}
			av_image_copy(dest_data, dest_linesizes, source_data, source_linesizes, static_cast<AVPixelFormat>(source->format), source->width, source->height / 2);
			for (int plane = 0; plane < AV_NUM_DATA_POINTERS; plane++)
				dest_data[plane] += dest->linesize[plane];
			av_image_copy(dest_data, dest_linesizes, source_data, source_linesizes, static_cast<AVPixelFormat>(source->format), source->width, source->height / 2);
			return dest;

		}
	}
}