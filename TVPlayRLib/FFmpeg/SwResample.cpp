#include "../pch.h"
#include "SwResample.h"

namespace TVPlayR {
	namespace FFmpeg {

		unique_ptr<SwrContext> AllocSwr(const AVChannelLayout& out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, const AVChannelLayout& in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate)
		{
			SwrContext* native_ctx = nullptr;
			swr_alloc_set_opts2(&native_ctx, &out_ch_layout, out_sample_fmt, out_sample_rate, &in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
			return unique_ptr<SwrContext>(native_ctx, [](SwrContext* ctx) { swr_free(&ctx); });
		}

		SwResample::SwResample(int src_channel_count, int src_sample_rate, AVSampleFormat src_sample_format, int dest_channel_count, int dest_sample_rate, AVSampleFormat dest_sample_format)
			: dest_sample_format_(dest_sample_format)
			, dest_sample_rate_(dest_sample_rate)
			, dest_channel_layout_(GetChannelLayout(dest_channel_count))
			, dest_time_base_(av_make_q(1, dest_sample_rate))
			, swr_(AllocSwr(dest_channel_layout_, dest_sample_format, dest_sample_rate, GetChannelLayout(src_channel_count), src_sample_format, src_sample_rate))
		{ }

		std::shared_ptr<AVFrame> SwResample::Resample(const std::shared_ptr<AVFrame> &frame)
		{
			std::shared_ptr<AVFrame> resampled = AllocFrame();
			resampled->nb_samples = swr_get_out_samples(swr_.get(), frame->nb_samples);
			resampled->format = dest_sample_format_;
			resampled->sample_rate = dest_sample_rate_;
			resampled->ch_layout = dest_channel_layout_;
			THROW_ON_FFMPEG_ERROR(swr_convert_frame(swr_.get(), resampled.get(), frame.get()));
			resampled->time_base = dest_time_base_;
			// TODO: verify this calculation
			resampled->pts = av_rescale_q(frame->pts, dest_time_base_, frame->time_base);
			return resampled;
		}

	}
}