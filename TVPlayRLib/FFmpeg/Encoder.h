#pragma once
#include "../Common/Executor.h"

namespace TVPlayR {
	namespace Core {
		class VideoFormat;
		enum class PixelFormat;
	}
	namespace FFmpeg {
		class OutputFormat;
	class Encoder
	{
	private:
		AVCodec* const encoder_;
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> enc_ctx_;
		std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>> fifo_;
		AVStream* const stream_;
		int audio_frame_size_ = 0;
		int64_t output_timestamp_ = 0LL;
		std::deque<std::shared_ptr<AVFrame>> frame_buffer_;
		Common::Executor executor_;
		std::mutex mutex_;
		void OpenCodec(const OutputFormat& formatContext);
		void InternalPush(const std::shared_ptr<AVFrame>& frame);
		std::shared_ptr<AVFrame> GetFrameFromFifo(int nb_samples);
	public:
		Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, const Core::VideoFormat& video_format, Core::PixelFormat pixel_format);
		Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, AVSampleFormat sample_format, int audio_sample_rate, int audio_channels_count);
		void Push(const std::shared_ptr<AVFrame>& frame);
		void Flush();
		std::shared_ptr<AVPacket> Pull();
	};


}}
