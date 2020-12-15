#include "../pch.h"
#include "Utils.h"
#include "Decoder.h"
#include "AudioFifo.h"

namespace TVPlayR {
	namespace FFmpeg {

Decoder::Decoder(const AVCodec* codec, AVStream* const stream, Core::HwAccel acceleration, const std::string& hw_device_index)
	: ctx(codec ? avcodec_alloc_context3(codec) : NULL, [](AVCodecContext* ctx) { if (ctx) 	avcodec_free_context(&ctx); })
	, start_ts(stream ? stream->start_time : 0)
	, stream_index(stream ? stream->index : 0)
	, channels_count(stream ? stream->codecpar->channels : 0)
	, sample_rate(stream ? stream->codecpar->sample_rate : 0)
	, frame_rate(stream ? stream->r_frame_rate : av_make_q(0, 1))
	, stream(stream)
	, is_eof_(false)
	, acceleration_(acceleration)
	, hw_device_index_(hw_device_index)
{
	if (!ctx || !stream)
		return;
	THROW_ON_FFMPEG_ERROR(avcodec_parameters_to_context(ctx.get(), stream->codecpar));
	if (acceleration != Core::HwAccel::none && (codec->id == AV_CODEC_ID_H264 || codec->id == AV_CODEC_ID_HEVC))
	{
		AVHWDeviceType device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
		AVPixelFormat hw_pix_format = AV_PIX_FMT_NONE;
		switch (acceleration)
		{
		case Core::HwAccel::cuvid:
			device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA;
			break;
		}
		const AVCodecHWConfig *config = NULL;
		for (int i = 0;; i++) {
			config = avcodec_get_hw_config(codec, i);
			if (!config) {
				break;
			}
			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
				config->device_type == device_type) {
				hw_pix_format = config->pix_fmt;
				break;
			}
		}
		if (hw_pix_format == AV_PIX_FMT_CUDA)
		{
			ctx->get_format = [](AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
			{
				const AVPixelFormat *p;
				for (p = pix_fmts; *p != -1; p++)
				{
					if (*p == AV_PIX_FMT_CUDA)
						return *p;
				}
				return AV_PIX_FMT_NONE;
			};
		}
		AVBufferRef* weak_hw_device_ctx = NULL;
		if (FF(av_hwdevice_ctx_create(&weak_hw_device_ctx, device_type, hw_device_index_.c_str(), NULL, 0)))
		{
			hw_device_ctx_ = AVBufferRefPtr(weak_hw_device_ctx, [](AVBufferRef* p) { av_buffer_unref(&p); });
			ctx->hw_device_ctx = av_buffer_ref(weak_hw_device_ctx);
		}
	}

	av_opt_set_int(ctx.get(), "refcounted_frames", 1, 0);
	av_opt_set_int(ctx.get(), "threads", 4, 0);
	THROW_ON_FFMPEG_ERROR(avcodec_open2(ctx.get(), codec, NULL));
}

Decoder::Decoder(const AVCodec * codec, AVStream * const stream, const int64_t fifo_duration)
	: Decoder(codec, stream, Core::HwAccel::none, "")
{
	fifo_.reset(new AudioFifo(ctx->sample_fmt, channels_count, sample_rate, stream->time_base, seek_time_, fifo_duration));
}

Decoder::~Decoder()
{
}

void Decoder::PushPacket(AVPacketPtr packet)
{
	if (!ctx)
		return;
	while (true)
	{
		int ret = avcodec_send_packet(ctx.get(), packet.get());
		switch (ret)
		{
		case 0:
			ReadFrames();
			return;
		case AVERROR(EAGAIN):
		{
			ReadFrames();
			continue;
		}
		default:
			THROW_ON_FFMPEG_ERROR(ret);
		}
	}
}

void Decoder::ReadFrames()
{
	while (true)
	{
		auto frame = AllocFrame();
		auto ret = avcodec_receive_frame(ctx.get(), frame.get());
		switch (ret)
		{
		case 0:
			if (frame->pts == AV_NOPTS_VALUE)
				frame->pts = frame->best_effort_timestamp;
			if (frame->pts != AV_NOPTS_VALUE)
				frame->pts -= start_ts; //normalize to zero
			if (fifo_)
				CopyPushFrame(frame);
			else
			{
				int64_t time = TimeFromTs(frame->pts);
				if (time >= seek_time_)
					CopyPushFrame(frame);
			}
			break;
		case AVERROR_EOF:
			is_eof_ = true;
			return;
		case AVERROR(EAGAIN):
			return;
		default:
			THROW_ON_FFMPEG_ERROR(ret);
		}
	}
}

void Decoder::CopyPushFrame(AVFramePtr & frame)
{
	if (hw_device_ctx_)
	{
		auto sw_frame = AllocFrame();
		THROW_ON_FFMPEG_ERROR(av_hwframe_transfer_data(sw_frame.get(), frame.get(), 0));
		sw_frame->pts = frame->pts;
		sw_frame->pict_type = frame->pict_type;
		frame_buffer_.push_back(sw_frame);
	}
	else
		frame_buffer_.push_back(frame);
}

bool Decoder::TryFeedFifo(AVFramePtr frame)
{
	return fifo_->TryPush(frame);
}

void Decoder::Flush()
{
	if (is_flushed_)
		return;
	PushPacket(__nullptr);
	is_flushed_ = true;
}

AVFramePtr Decoder::PullVideo()
{
	if (frame_buffer_.empty())
		return __nullptr;
	auto frame = frame_buffer_.front();
	frame_buffer_.pop_front();
	//OutputDebugStringA(("Pulled frame from decoder: " + std::to_string(TimeFromTs(frame->pts) / 1000) + "\n").c_str());
	return frame;
}

AVFramePtr Decoder::PullAudio()
{
	while (!frame_buffer_.empty())
	{
		if (fifo_->TryPush(frame_buffer_.front()))
			frame_buffer_.pop_front();
		else
			break;
	}
	return fifo_->Pull((std::numeric_limits<int>::max)());
}

void Decoder::Seek(const int64_t seek_time)
{
	avcodec_flush_buffers(ctx.get());
	frame_buffer_.clear();
	if (fifo_)
	{
		fifo_->Reset(seek_time);
	}
	is_eof_ = false;
	is_flushed_ = false;
	seek_time_ = seek_time;
}

int64_t Decoder::TimeMin()
{
	if (fifo_)
		return fifo_->TimeMin();
	if (frame_buffer_.empty())
		return 0LL;
	return TimeFromTs(frame_buffer_.front()->pts);
}

int64_t Decoder::TimeMax()
{
	if (frame_buffer_.empty())
	{
		if (fifo_)
			fifo_->TimeMax();
		return 0LL;
	}
	return TimeFromTs(frame_buffer_.back()->pts);
}

int64_t Decoder::FrontFrameEndTime() const
{
	if (frame_buffer_.empty())
		return 0LL;
	return TimeFromTs(frame_buffer_.front()->pts + frame_buffer_.front()->pkt_duration);
}

int64_t Decoder::TimeFromTs(int64_t ts) const
{
	return av_rescale_q(ts, stream->time_base, av_get_time_base_q());
}

bool Decoder::IsEof() const
{
	return is_eof_ && frame_buffer_.empty() && (!fifo_ || fifo_->SamplesCount() == 0);
}

bool Decoder::Empty() const
{
	return frame_buffer_.empty() && (!fifo_ || fifo_->SamplesCount() == 0);
}

}}