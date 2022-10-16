#include "../pch.h"
#include "SwResample.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

		std::unique_ptr<SwrContext, std::function<void(SwrContext*)>> AllocSwr(int out_ch_count, enum AVSampleFormat out_sample_fmt, int out_sample_rate,int in_ch_count, enum AVSampleFormat in_sample_fmt, int in_sample_rate)
		{
			SwrContext* native_ctx = nullptr;
			AVChannelLayout in_ch_layout, out_ch_layout;
			av_channel_layout_default(&in_ch_layout, in_ch_count);
			av_channel_layout_default(&out_ch_layout, out_ch_count);
			swr_alloc_set_opts2(&native_ctx, &out_ch_layout, out_sample_fmt, out_sample_rate, &in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
			return std::unique_ptr<SwrContext, std::function<void(SwrContext*)>>(native_ctx, [](SwrContext* ctx) { swr_free(&ctx); });
		}

		SwResample::SwResample(int src_channel_count, int src_sample_rate, AVSampleFormat src_sample_format, int dest_channel_count, int dest_sample_rate, AVSampleFormat dest_sample_format)
			: dest_sample_format_(dest_sample_format)
			, dest_sample_rate_(dest_sample_rate)
			, swr_(AllocSwr(dest_channel_count, dest_sample_format, dest_sample_rate, src_channel_count, src_sample_format, src_sample_rate))
		{ 
			av_channel_layout_default(&dest_channel_layout_, dest_channel_count);
		}

		std::shared_ptr<AVFrame> SwResample::Resample(const std::shared_ptr<AVFrame> frame)
		{
			std::shared_ptr<AVFrame> resampled = AllocFrame();
			resampled->nb_samples = swr_get_out_samples(swr_.get(), frame->nb_samples);
			resampled->format = dest_sample_format_;
			resampled->sample_rate = dest_sample_rate_;
			resampled->ch_layout = dest_channel_layout_;
			THROW_ON_FFMPEG_ERROR(swr_convert_frame(swr_.get(), resampled.get(), frame.get()));
			resampled->pts = frame->pts;
			return resampled;
		}

	}
}