#include "../pch.h"
#include "Encoder.h"
#include "FFmpegUtils.h"
#include "OutputFormat.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace FFmpeg {

	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> GetContext(const AVCodec* encoder)
	{
		if (!encoder)
		{
			OutputDebugString(L"Encoder not found");
			return __nullptr;
		}
		AVCodecContext * ctx = avcodec_alloc_context3(encoder);
		if (!ctx)
			return __nullptr;
		return std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(ctx, [](AVCodecContext* c)
		{
			avcodec_free_context(&c);
		});
	}

	Encoder::Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, const Core::VideoFormat& video_format, Core::PixelFormat pixel_format)
		: executor_("Video encoder")
		, encoder_(encoder)
		, enc_ctx_(GetContext(encoder))
		, stream_(avformat_new_stream(output_format.Ctx().get(), NULL))
	{
		auto enc = enc_ctx_.get();
		enc->height = video_format.height();
		enc->width = video_format.width();
		enc->sample_aspect_ratio = video_format.SampleAspectRatio().av();
		enc->pix_fmt = Core::PixelFormatToFFmpegFormat(pixel_format);
		enc->framerate = video_format.FrameRate().av();
		enc->time_base = video_format.FrameRate().invert().av();
		stream_->sample_aspect_ratio = enc->sample_aspect_ratio;
		
		if (encoder->id == AV_CODEC_ID_MJPEG)
		{
			enc_ctx_->color_range = AVCOL_RANGE_JPEG;
			enc_ctx_->qmax = 2;
		}
		else
		{
			enc_ctx_->gop_size = 50;
			enc_ctx_->bit_rate = bitrate;
			enc_ctx_->max_b_frames = 3;
		}
		if (output_format.Ctx()->oformat->flags & AVFMT_GLOBALHEADER)
			enc_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		OpenCodec(output_format);
	}


	Encoder::Encoder(const OutputFormat& output_format, AVCodec* const encoder, int bitrate, AVSampleFormat sample_format, int audio_sample_rate, int audio_channels_count)
		: executor_("Audio encoder")
		, encoder_(encoder)
		, enc_ctx_(GetContext(encoder))
		, stream_(avformat_new_stream(output_format.Ctx().get(), NULL))
	{
		enc_ctx_->sample_rate = audio_sample_rate;
		enc_ctx_->channel_layout = av_get_default_channel_layout(audio_channels_count);
		enc_ctx_->channels = audio_channels_count;
		enc_ctx_->sample_fmt = sample_format;
		enc_ctx_->time_base = av_make_q(1, audio_sample_rate);
		enc_ctx_->bit_rate = bitrate;
		OpenCodec(output_format);		
		if (enc_ctx_->frame_size > 0)
		{
			audio_frame_size_ = enc_ctx_->frame_size;
			fifo_ = std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>>(av_audio_fifo_alloc(sample_format, audio_channels_count, audio_sample_rate / 10), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); });
		}
	}

	void Encoder::OpenCodec(const OutputFormat& otput_format)
	{
		THROW_ON_FFMPEG_ERROR(avcodec_open2(enc_ctx_.get(), encoder_, NULL));
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_from_context(stream_->codecpar, enc_ctx_.get()))
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