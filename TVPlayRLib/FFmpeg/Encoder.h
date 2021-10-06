#pragma once


namespace TVPlayR {
	namespace Core {
		class VideoFormat;
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
		void OpenCodec(const OutputFormat& formatContext);
		void InternalPush(const std::shared_ptr<AVFrame>& frame);
	public:
		Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, const Core::VideoFormat& video_format, Core::PixelFormat pixel_format);
		Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, AVSampleFormat sample_format, int audio_sample_rate, int audio_channels_count);
		void Push(const std::shared_ptr<AVFrame>& frame);
		void Flush();
		std::shared_ptr<AVPacket> Pull();
	};


}}
