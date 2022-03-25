#include "../pch.h"
#include "SwResample.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

		SwResample::SwResample(int src_channels, int src_sample_rate, AVSampleFormat src_sample_format, int dest_channels, int dest_sample_rate, AVSampleFormat dest_sample_format)
			: dest_channel_layout_(av_get_default_channel_layout(dest_channels))
			, dest_channels_(dest_channels)
			, dest_sample_format_(dest_sample_format)
			, dest_sample_rate_(dest_sample_rate)
			, swr_(swr_alloc_set_opts(NULL, dest_channel_layout_, dest_sample_format, dest_sample_rate, av_get_default_channel_layout(src_channels), src_sample_format, src_sample_rate, 0, NULL), [](SwrContext* ctx) { swr_free(&ctx); })
		{ }

		std::shared_ptr<AVFrame> SwResample::Resample(const std::shared_ptr<AVFrame> frame)
		{
			std::shared_ptr<AVFrame> resampled = AllocFrame();
			resampled->nb_samples = swr_get_out_samples(swr_.get(), frame->nb_samples);
			resampled->format = dest_sample_format_;
			resampled->sample_rate = dest_sample_rate_;
			resampled->channels = dest_channels_;
			resampled->channel_layout = dest_channel_layout_;
			THROW_ON_FFMPEG_ERROR(swr_convert_frame(swr_.get(), resampled.get(), frame.get()));
			resampled->pts = frame->pts;
			return resampled;
		}

	}
}