#include "../pch.h"
#include "Encoder.h"
#include "FFmpegUtils.h"
#include "OutputFormat.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"

namespace TVPlayR {
	namespace FFmpeg {

	Encoder::Encoder(const OutputFormat& output_format, const AVCodec* encoder, int bitrate, std::shared_ptr<AVFrame> video_frame, AVRational time_base, AVRational frame_rate, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(Common::DebugSeverity::info, "Video encoder for " + output_format.GetUrl())
		, encoder_(encoder)
		, enc_ctx_(GetVideoContext(output_format.Ctx(), encoder_, bitrate, video_frame->width, video_frame->height, time_base, frame_rate, video_frame->sample_aspect_ratio, video_frame->interlaced_frame))
		, format_(enc_ctx_->pix_fmt)
	{
		OpenCodec(output_format.Ctx(), options, stream_metadata, stream_id);
	}


	Encoder::Encoder(const OutputFormat& output_format, const AVCodec* encoder, int bitrate, int audio_sample_rate, AVChannelLayout& audio_channel_layout, AVDictionary** options, const std::string& stream_metadata, int stream_id)
		: Common::DebugTarget(Common::DebugSeverity::info, "Audio encoder for " + output_format.GetUrl())
		, encoder_(encoder)
		, enc_ctx_(GetAudioContext(output_format.Ctx(), encoder_, bitrate, audio_sample_rate, audio_channel_layout))
		, format_(enc_ctx_->sample_fmt)
	{
		OpenCodec(output_format.Ctx(), options, stream_metadata, stream_id);
		if (enc_ctx_->frame_size > 0)
		{
			audio_frame_size_ = enc_ctx_->frame_size;
			fifo_ = std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>>(av_audio_fifo_alloc(encoder_->sample_fmts[0], audio_channel_layout.nb_channels, audio_frame_size_ * 3), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); });
		}
	}

	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetAudioContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, int sample_rate, AVChannelLayout& audio_channel_layout)
	{
		if (!encoder)
		{
			DebugPrintLine(Common::DebugSeverity::error, "Encoder not found");
			return nullptr;
		}
		AVCodecContext* ctx = avcodec_alloc_context3(encoder);
		if (!ctx)
			return nullptr;
		ctx->sample_rate = sample_rate;
		av_channel_layout_copy(&ctx->ch_layout, &audio_channel_layout);
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
	
	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> Encoder::GetVideoContext(AVFormatContext* const format_context, const AVCodec* encoder, int bitrate, int width, int height, AVRational time_base, AVRational frame_rate, AVRational sar, bool interlaced)
	{
		if (!encoder)
		{
			DebugPrintLine(Common::DebugSeverity::error, "Encoder not found");
			return nullptr;
		}
		AVCodecContext* ctx = avcodec_alloc_context3(encoder);
		if (!ctx)
			return nullptr;
		ctx->height = height;
		ctx->width = width;
		ctx->sample_aspect_ratio = sar;
		ctx->pix_fmt = encoder->pix_fmts[0];
		ctx->framerate = frame_rate;
		ctx->time_base = time_base;
		ctx->bit_rate = bitrate * 1000;

		if (interlaced)
			ctx->flags |= (AV_CODEC_FLAG_INTERLACED_ME | AV_CODEC_FLAG_INTERLACED_DCT);

		if (ctx->codec_id == AV_CODEC_ID_PRORES)
		{
			ctx->bit_rate = ctx->width < 1280 ? 63 * 1000000 : 220 * 1000000;
			ctx->pix_fmt = AV_PIX_FMT_YUV422P10;
		}
		else if (ctx->codec_id == AV_CODEC_ID_DNXHD)
		{
			if (ctx->width < 1280 || ctx->height < 720)
				THROW_EXCEPTION("Encoder: unsupported video dimensions.");
			ctx->bit_rate = 220 * 1000000;
			ctx->pix_fmt = AV_PIX_FMT_YUV422P;
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
		stream_->time_base = enc_ctx_->time_base;
		stream_->sample_aspect_ratio = enc_ctx_->sample_aspect_ratio;
		stream_->avg_frame_rate = enc_ctx_->framerate;
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_from_context(stream_->codecpar, enc_ctx_.get()));
	}
	
	void Encoder::Push(const std::shared_ptr<AVFrame>& frame)
	{
		if (fifo_)
		{
			if (frame->nb_samples > av_audio_fifo_space(fifo_.get()))
				THROW_ON_FFMPEG_ERROR(av_audio_fifo_realloc(fifo_.get(), frame->nb_samples * 2));
			if (av_audio_fifo_write(fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
				THROW_EXCEPTION("Encoder: can't write all samples to audio fifo");
			while (av_audio_fifo_size(fifo_.get()) >= audio_frame_size_)
				frame_buffer_.emplace_back(GetFrameFromFifo(audio_frame_size_));
		}
		else
		{
			frame->pict_type = AV_PICTURE_TYPE_NONE;
			frame->pts = output_timestamp_;
			output_timestamp_ += (enc_ctx_->codec_type == AVMEDIA_TYPE_AUDIO ? frame->nb_samples : 1LL);
			frame_buffer_.emplace_back(frame);
		}
	}

	bool Encoder::InternalPush(AVFrame* frame)
	{
		if (frame)
			DebugPrintLine(Common::DebugSeverity::trace, "InternalPush pts=" + std::to_string(frame->pts));
		else
			DebugPrintLine(Common::DebugSeverity::debug, "InternalPush flush frame");
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
		frame->ch_layout = enc_ctx_->ch_layout;
		frame->pts = output_timestamp_;
		output_timestamp_ += nb_samples;
		THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 0));
		if (int readed = av_audio_fifo_read(fifo_.get(), (void**)frame->data, audio_frame_size_) < nb_samples)
			THROW_EXCEPTION("Encoder: readed too few samples from av_audio_fifo_read()");
		return frame;
	}

	void Encoder::Flush()
	{
		if (fifo_)
		{
			int fifo_samples = av_audio_fifo_size(fifo_.get());
			assert(fifo_samples < audio_frame_size_);
			if (fifo_samples > 0)
				frame_buffer_.emplace_back(GetFrameFromFifo(fifo_samples));
		}
		frame_buffer_.emplace_back(nullptr);
	}

	std::shared_ptr<AVPacket> Encoder::Pull()
	{
		while (true)
		{
			std::shared_ptr<AVPacket> packet = AllocPacket();
			auto ret = avcodec_receive_packet(enc_ctx_.get(), packet.get());
			switch (ret)
			{
			case 0:
				av_packet_rescale_ts(packet.get(), enc_ctx_->time_base, stream_->time_base);
				packet->stream_index = stream_->index;
				DebugPrintLine(Common::DebugSeverity::trace, "Pull packet stream=" + std::to_string(packet->stream_index) + ", time=" + std::to_string(PtsToTime(packet->pts, stream_->time_base)/1000));
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