#include "../pch.h"
#include "Encoder.h"
#include "FFmpegUtils.h"
#include "OutputFormat.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace FFmpeg {


	Encoder::Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, const Core::VideoFormat& video_format, TVPlayR::PixelFormat pixel_format, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(true, "Video encoder for " + output_format.GetFileName())
		, executor_("Video encoder for " + output_format.GetFileName())
		, encoder_(avcodec_find_encoder_by_name(encoder.c_str()))
		, format_ctx_(output_format.Ctx().get())
		, enc_ctx_(GetVideoContext(encoder_, bitrate, video_format, pixel_format))
	{
		OpenCodec(output_format, options, stream_metadata, stream_id);
	}


	Encoder::Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, AVSampleFormat sample_format, int audio_sample_rate, int audio_channels_count, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(true, "Audio encoder for " + output_format.GetFileName())
		, executor_("Audio encoder for " + output_format.GetFileName())
		, encoder_(avcodec_find_encoder_by_name(encoder.c_str()))
		, format_ctx_(output_format.Ctx().get())
		, enc_ctx_(GetAudioContext(encoder_, bitrate, audio_sample_rate, audio_channels_count, sample_format))
	{
		OpenCodec(output_format, options, stream_metadata, stream_id);
		if (enc_ctx_->frame_size > 0)
		{
			audio_frame_size_ = enc_ctx_->frame_size;
			fifo_ = std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>>(av_audio_fifo_alloc(sample_format, audio_channels_count, audio_sample_rate / 10), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); });
		}
	}

	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetAudioContext(const AVCodec* encoder, int bitrate, int sample_rate, int channels_count, AVSampleFormat sample_format)
	{
		if (!encoder)
		{
			DebugPrintLine("Encoder not found");
			return nullptr;
		}
		AVCodecContext* ctx = avcodec_alloc_context3(encoder);
		if (!ctx)
			return nullptr;
		ctx->sample_rate = sample_rate;
		ctx->channel_layout = av_get_default_channel_layout(channels_count);
		ctx->channels = channels_count;
		ctx->sample_fmt = sample_format;
		ctx->time_base = av_make_q(1, sample_rate);
		ctx->bit_rate = bitrate;
		if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
			ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		return std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(ctx, [](AVCodecContext* c)
		{
			avcodec_free_context(&c);
		});
	}
	
	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetVideoContext(const AVCodec* encoder, int bitrate, const Core::VideoFormat& video_format, TVPlayR::PixelFormat pixel_format)
	{
		if (!encoder)
		{
			DebugPrintLine("Encoder not found");
			return nullptr;
		}
		AVCodecContext* ctx = avcodec_alloc_context3(encoder);
		if (!ctx)
			return nullptr;
		ctx->height = video_format.height();
		ctx->width = video_format.width();
		ctx->sample_aspect_ratio = video_format.SampleAspectRatio().av();
		ctx->pix_fmt = TVPlayR::PixelFormatToFFmpegFormat(pixel_format);
		ctx->framerate = video_format.FrameRate().av();
		ctx->time_base = av_inv_q(ctx->framerate);
		ctx->max_b_frames = 0; // b-frames not supported by default.
		ctx->bit_rate = bitrate;

		if (video_format.interlaced())
			ctx->flags |= (AV_CODEC_FLAG_INTERLACED_ME | AV_CODEC_FLAG_INTERLACED_DCT);

		if (ctx->codec_id == AV_CODEC_ID_PRORES)
		{
			ctx->bit_rate = ctx->width < 1280 ? 63 * 1000000 : 220 * 1000000;
			ctx->pix_fmt = AV_PIX_FMT_YUV422P10;
		}
		else if (ctx->codec_id == AV_CODEC_ID_DNXHD)
		{
			if (ctx->width < 1280 || ctx->height < 720)
				THROW_EXCEPTION("Unsupported video dimensions.");
			ctx->bit_rate = 220 * 1000000;
			ctx->pix_fmt = AV_PIX_FMT_YUV422P;
		}
		else if (ctx->codec_id == AV_CODEC_ID_DVVIDEO)
		{
			ctx->width = ctx->height == 1280 ? 960 : ctx->width;
			if (video_format.type() == Core::VideoFormatType::ntsc || video_format.type() == Core::VideoFormatType::ntsc_fha)
				ctx->pix_fmt = AV_PIX_FMT_YUV411P;
			else if (video_format.type() == Core::VideoFormatType::pal || video_format.type() == Core::VideoFormatType::pal_fha)
				ctx->pix_fmt = AV_PIX_FMT_YUV420P;
			else // dv50
				ctx->pix_fmt = AV_PIX_FMT_YUV422P;
			if (video_format.FrameRate().Denominator() == 1001)
				ctx->width = ctx->height == 1080 ? 1280 : ctx->width;
			else
				ctx->width = ctx->height == 1080 ? 1440 : ctx->width;
		}
		else if (ctx->codec_id == AV_CODEC_ID_H264)
		{
			ctx->gop_size = 30;
			ctx->max_b_frames = 2;
		}
		else if (ctx->codec_id == AV_CODEC_ID_QTRLE)
		{
			ctx->pix_fmt = AV_PIX_FMT_ARGB;
		}

		if (encoder_->id == AV_CODEC_ID_MJPEG)
		{
			ctx->color_range = AVCOL_RANGE_JPEG;
			ctx->qmax = 2;
		}
		else
		{
			ctx->gop_size = 50;
			ctx->bit_rate = bitrate;
			ctx->max_b_frames = 3;
		}
		if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
			ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;		
		return std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(ctx, [](AVCodecContext* c)
		{
			avcodec_free_context(&c);
		});
	}

	void Encoder::OpenCodec(const OutputFormat& otput_format, AVDictionary** options, const std::string& stream_metadata, int stream_id)
	{
		THROW_ON_FFMPEG_ERROR(avcodec_open2(enc_ctx_.get(), encoder_, options));
		stream_ = avformat_new_stream(format_ctx_, encoder_);
		stream_->metadata = ReadOptions(stream_metadata);
		stream_->id = stream_id;
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_from_context(stream_->codecpar, enc_ctx_.get()));
		THROW_ON_FFMPEG_ERROR(avformat_write_header(format_ctx_, options));
	}
	
	void Encoder::Push(const std::shared_ptr<AVFrame>& frame)
	{
		executor_.begin_invoke([=]
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (!fifo_)
			{
				InternalPush(frame);
				return;
			}
			if (frame->nb_samples > av_audio_fifo_space(fifo_.get()))
				THROW_ON_FFMPEG_ERROR(av_audio_fifo_realloc(fifo_.get(), frame->nb_samples * 2));
			if (av_audio_fifo_write(fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
				THROW_EXCEPTION("Can't write all samples to audio fifo");
			while (av_audio_fifo_size(fifo_.get()) >= audio_frame_size_)
			{
				auto frame_from_fifo = GetFrameFromFifo(audio_frame_size_);
				InternalPush(frame_from_fifo);
			}
		});
	}

	void Encoder::InternalPush(const std::shared_ptr<AVFrame>& frame)
	{
		output_timestamp_ += (enc_ctx_->codec_type == AVMEDIA_TYPE_AUDIO ? frame->nb_samples : 1LL);
		frame->pict_type = AV_PICTURE_TYPE_NONE;
		if (enc_ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
			frame->key_frame = 0;
		frame->pts = output_timestamp_;
		int ret = avcodec_send_frame(enc_ctx_.get(), frame.get());
		switch (ret)
		{
		case AVERROR(EAGAIN):
			frame_buffer_.push_back(frame);
			break;
		default:
			break;
		}
	}

	std::shared_ptr<AVFrame> Encoder::GetFrameFromFifo(int nb_samples)
	{
		auto frame = FFmpeg::AllocFrame();
		frame->nb_samples = nb_samples;
		frame->format = enc_ctx_->sample_fmt;
		frame->channels = enc_ctx_->channels;
		frame->channel_layout = enc_ctx_->channel_layout;
		THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 0));
		if (int readed = av_audio_fifo_read(fifo_.get(), (void**)frame->data, audio_frame_size_) < nb_samples)
			THROW_EXCEPTION("Readed too few frames");
		return frame;
	}

	void Encoder::SetVideoParameters(AVCodecContext* context)
	{
	}

	void Encoder::Flush()
	{
		executor_.begin_invoke([this] 
		{
			std::lock_guard<std::mutex> lock(mutex_);
			THROW_ON_FFMPEG_ERROR(avcodec_send_frame(enc_ctx_.get(), NULL));
		});
	}

	std::shared_ptr<AVPacket> Encoder::Pull()
	{
		auto packet(AllocPacket());
		while (true)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			auto ret = avcodec_receive_packet(enc_ctx_.get(), packet.get());
			switch (ret)
			{
			case 0:
				packet->stream_index = stream_->index;
				av_packet_rescale_ts(packet.get(), enc_ctx_->time_base, stream_->time_base);
				return packet;
			case AVERROR(EAGAIN):
				if (!frame_buffer_.empty())
				{
					auto frame = frame_buffer_.front();
					frame_buffer_.pop_front();
					THROW_ON_FFMPEG_ERROR(avcodec_send_frame(enc_ctx_.get(), frame.get()));
					continue;
				}
				return nullptr;
			default:
				return nullptr;
			}
		}
		return nullptr;
	}
}}