#pragma once
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
		class SwResample: Common::NonCopyable
		{
		public:
			SwResample(int src_channel_count, int src_sample_rate, AVSampleFormat src_sample_format, int dest_channel_count , int dest_sample_rate, AVSampleFormat dest_sample_format);
			std::shared_ptr<AVFrame> Resample(const std::shared_ptr<AVFrame> &frame);
			int OutputSampleRate() const { return dest_sample_rate_; }
			AVChannelLayout OutputChannelLayout() const { return dest_channel_layout_; }
		private:
			const int dest_sample_rate_;
			const AVRational dest_time_base_;
			const AVSampleFormat dest_sample_format_;
			const AVChannelLayout dest_channel_layout_;
			unique_ptr<SwrContext> swr_;
		};

}}

