#pragma once
#include "../Common/Debug.h"
#include "../Common/Executor.h"

namespace TVPlayR {
		enum class PixelFormat;
	namespace Core {
		class VideoFormat;
	}
	namespace FFmpeg {
		class OutputFormat;
	class Encoder : private Common::DebugTarget
	{
	private:
		AVCodec* const encoder_;
		AVFormatContext* const format_ctx_;
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> enc_ctx_;
		std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>> fifo_;
		AVStream* stream_;
		int audio_frame_size_ = 0;
		int64_t output_timestamp_ = 0LL;
		std::deque<std::shared_ptr<AVFrame>> frame_buffer_;
		Common::Executor executor_;
		std::mutex mutex_;
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> GetAudioContext(const AVCodec* encoder, int bitrate, int sample_rate, int channels_count, AVSampleFormat sample_format);
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> GetVideoContext(const AVCodec* encoder, int bitrate, const Core::VideoFormat& video_format, TVPlayR::PixelFormat pixel_format);
		void OpenCodec(const OutputFormat& formatContext, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		void InternalPush(const std::shared_ptr<AVFrame>& frame);
		std::shared_ptr<AVFrame> GetFrameFromFifo(int nb_samples);
		void SetVideoParameters(AVCodecContext* context);
	public:
		Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, const Core::VideoFormat& video_format, TVPlayR::PixelFormat pixel_format, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, AVSampleFormat sample_format, int audio_sample_rate, int audio_channels_count, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		void Push(const std::shared_ptr<AVFrame>& frame);
		void Flush();
		std::shared_ptr<AVPacket> Pull();
	};


}}
