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
		const AVCodec* encoder_;
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> enc_ctx_;
		std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>> fifo_;
		AVStream* stream_;
		int audio_frame_size_ = 0;
		int64_t output_timestamp_ = 0LL;
		std::deque<std::shared_ptr<AVFrame>> frame_buffer_;
		std::mutex mutex_;
		const int format_;
		const int sample_rate_;
		bool flushed_ = false;
		bool is_eof_ = false;
		Common::Executor executor_;
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> GetAudioContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, int sample_rate, int channels_count);
		std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> GetVideoContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, const Core::VideoFormat& video_format);
		void OpenCodec(AVFormatContext* const format_context, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		bool InternalPush(AVFrame* frame);
		std::shared_ptr<AVFrame> GetFrameFromFifo(int nb_samples);
	public:
		Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, const Core::VideoFormat& video_format, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, int audio_sample_rate, int audio_channels_count, AVDictionary** options, const std::string& stream_metadata, int stream_id);
		void Push(const std::shared_ptr<AVFrame>& frame);
		void Flush();
		bool IsEof() const { return is_eof_; }
		int Format() const { return format_; }
		int SampleRate() const { return sample_rate_; }
		int Width() const { return enc_ctx_->width; }
		int Height() const { return enc_ctx_->height; }
		std::shared_ptr<AVPacket> Pull();
	};


}}
