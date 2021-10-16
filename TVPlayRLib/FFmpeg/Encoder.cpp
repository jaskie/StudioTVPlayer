#include "../pch.h"
#include "Encoder.h"
#include "FFmpegUtils.h"
#include "OutputFormat.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace FFmpeg {

	Encoder::Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, const Core::VideoFormat& video_format, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(true, "Video encoder for " + output_format.GetUrl())
		, executor_("Video encoder for " + output_format.GetUrl())
		, encoder_(avcodec_find_encoder_by_name(encoder.c_str()))
		, enc_ctx_(GetVideoContext(output_format.Ctx(), encoder_, bitrate, video_format))
		, format_(enc_ctx_->pix_fmt)
		, sample_rate_(0)
	{
		OpenCodec(output_format.Ctx(), options, stream_metadata, stream_id);
	}


	Encoder::Encoder(const OutputFormat& output_format, const std::string& encoder, int bitrate, int audio_sample_rate, int audio_channels_count, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(true, "Audio encoder for " + output_format.GetUrl())
		, executor_("Audio encoder for " + output_format.GetUrl(), 2)
		, encoder_(avcodec_find_encoder_by_name(encoder.c_str()))
		, enc_ctx_(GetAudioContext(output_format.Ctx(), encoder_, bitrate, audio_sample_rate, audio_channels_count))
		, format_(enc_ctx_->sample_fmt)
		, sample_rate_(enc_ctx_->sample_rate)
	{
		OpenCodec(output_format.Ctx(), options, stream_metadata, stream_id);
		if (enc_ctx_->frame_size > 0)
		{
			audio_frame_size_ = enc_ctx_->frame_size;
			fifo_ = std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>>(av_audio_fifo_alloc(encoder_->sample_fmts[0], audio_channels_count, audio_frame_size_ * 3), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); });
		}
	}

	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetAudioContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, int sample_rate, int channels_count)
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
		ctx->sample_fmt = encoder->sample_fmts[0];
		ctx->time_base = av_make_q(1, sample_rate);
		ctx->bit_rate = bitrate * 1000;
		if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
			ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		return std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(ctx, [](AVCodecContext* c)
		{
			avcodec_free_context(&c);
		});
	}
	
	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetVideoContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, const Core::VideoFormat& video_format)
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
		ctx->pix_fmt = encoder->pix_fmts[0];
		ctx->framerate = video_format.FrameRate().av();
		ctx->time_base = av_inv_q(ctx->framerate);
		ctx->max_b_frames = 0; // b-frames not supported by default.
		ctx->bit_rate = bitrate * 1000;

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
		else if (encoder_->id == AV_CODEC_ID_MJPEG)
		{
			ctx->color_range = AVCOL_RANGE_JPEG;
			ctx->qmax = 2;
		}

		if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
			ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;		
		return std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(ctx, [](AVCodecContext* c)
		{
			avcodec_free_context(&c);
		});
	}

	void Encoder::OpenCodec(AVFormatContext* const format_context, AVDictionary** options, const std::string& stream_metadata, int stream_id)
	{
		THROW_ON_FFMPEG_ERROR(avcodec_open2(enc_ctx_.get(), encoder_, options));
		stream_ = avformat_new_stream(format_context, encoder_);
		stream_->metadata = ReadOptions(stream_metadata);
		stream_->id = stream_id;
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_from_context(stream_->codecpar, enc_ctx_.get()));
	}
	
	void Encoder::Push(const std::shared_ptr<AVFrame>& frame)
	{
		executor_.begin_invoke([this, frame]
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (fifo_)
			{
				if (frame->nb_samples > av_audio_fifo_space(fifo_.get()))
					THROW_ON_FFMPEG_ERROR(av_audio_fifo_realloc(fifo_.get(), frame->nb_samples * 2));
				if (av_audio_fifo_write(fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
					THROW_EXCEPTION("Can't write all samples to audio fifo");
				while (av_audio_fifo_size(fifo_.get()) >= audio_frame_size_)
					frame_buffer_.emplace_back(GetFrameFromFifo(audio_frame_size_));
			}
			else
				frame_buffer_.emplace_back(av_frame_clone(frame.get()), [](AVFrame* ptr) { av_frame_free(&ptr); });
		});
	}

	bool Encoder::InternalPush(AVFrame* frame)
	{
		if (frame)
		{
			frame->pict_type = AV_PICTURE_TYPE_NONE;
			if (enc_ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				frame->key_frame = 0;
			frame->pts = output_timestamp_;
			output_timestamp_ += (enc_ctx_->codec_type == AVMEDIA_TYPE_AUDIO ? frame->nb_samples : 1LL);
			DebugPrintLine("InternalPush pts=" + std::to_string(output_timestamp_));
		}
		else
			DebugPrintLine("InternalPush flush frame");
		int ret = avcodec_send_frame(enc_ctx_.get(), frame);
		switch (ret)
		{
		case AVERROR(EAGAIN):
			return false;
			break;
		default:
			break;
		}
		return true;
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

	void Encoder::Flush()
	{
		executor_.begin_invoke([this] 
		{
			std::lock_guard<std::mutex> lock(mutex_);
			InternalPush(nullptr);
			flushed_ = true;
		});
	}

	std::shared_ptr<AVPacket> Encoder::Pull()
	{
		while (true)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			auto packet(AllocPacket());
			auto ret = avcodec_receive_packet(enc_ctx_.get(), packet.get());
			switch (ret)
			{
			case 0:
				av_packet_rescale_ts(packet.get(), enc_ctx_->time_base, stream_->time_base);
				DebugPrintLine("Pull packet stream=" + std::to_string(packet->stream_index) + ", time=" + std::to_string(PtsToTime(packet->pts, stream_->time_base)/1000));
				packet->stream_index = stream_->index;
				packet->time_base = stream_->time_base;
				return packet;
			case AVERROR(EAGAIN):
				if (!frame_buffer_.empty())
				{
					auto frame = frame_buffer_.front();
					if (InternalPush(frame.get()))
						frame_buffer_.pop_front();
					continue;
				}
				return nullptr;
			case AVERROR_EOF:
				is_eof_ = true;
				return nullptr;
			default:
				THROW_ON_FFMPEG_ERROR(ret);
			}
		}
		return nullptr;
	}
}}