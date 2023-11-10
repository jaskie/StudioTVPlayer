#pragma once

namespace TVPlayR {
	namespace FFmpeg {
		class SwResample: Common::NonCopyable
		{
		public:
			SwResample(int src_channel_count, int src_sample_rate, AVSampleFormat src_sample_format, int dest_channel_count , int dest_sample_rate, AVSampleFormat dest_sample_format);
			std::shared_ptr<AVFrame> Resample(const std::shared_ptr<AVFrame>& frame);
			int OutputSampleRate() const { return dest_sample_rate_; }
			AVChannelLayout OutputChannelLayout() const { return dest_channel_layout_; }
		private:
			const int dest_sample_rate_; 
			const AVSampleFormat dest_sample_format_;
			AVChannelLayout dest_channel_layout_;
			std::unique_ptr<SwrContext, std::function<void(SwrContext*)>> swr_;
		};

}}

