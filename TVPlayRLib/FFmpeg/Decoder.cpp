#include "../pch.h"
#include "FFmpegUtils.h"
#include "Decoder.h"
#include "AudioFifo.h"

namespace TVPlayR {
	namespace FFmpeg {

AVPixelFormat GetHwPixelFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
	const AVPixelFormat *p;
	for (p = pix_fmts; *p != -1; p++)
	{
		if (*p == AV_PIX_FMT_CUDA)
			return *p;
	}
	return AV_PIX_FMT_NONE;
}

struct Decoder::implementation : Common::DebugTarget
{
	const AVCodec *codec_;
	const std::int64_t start_ts_;
	const Core::HwAccel acceleration_;
	AVCodecHWConfig *hw_config_ = NULL;
	const std::string hw_device_index_;
	bool is_eof_ = false;
	bool flush_packet_sent_ = false;
	bool flush_packet_received_ = false;
	const int stream_index_;
	const int channels_count_;
	const int sample_rate_;
	std::deque<std::shared_ptr<AVPacket>> packet_queue_;
	const AVRational time_base_;
	AVStream * const stream_;
	const AVMediaType media_type_;
	unique_ptr<AVBufferRef> hw_device_ctx_;
	unique_ptr<AVCodecContext> ctx_;
	std::int64_t seek_pts_;
	const std::int64_t duration_;
	std::mutex mutex_;

	implementation(const AVCodec *codec, AVStream * const stream, std::int64_t seek_time, Core::HwAccel acceleration, const std::string &hw_device_index)
		: Common::DebugTarget(Common::DebugSeverity::info, "Decoder")
		, codec_(codec)
		, ctx_(CreateCodecContext())
		, start_ts_(stream ? stream->start_time : 0LL)
		, duration_(stream ? stream->duration : 0LL)
		, stream_index_(stream ? stream->index : 0)
		, channels_count_(stream&& stream->codecpar ? stream->codecpar->ch_layout.nb_channels : 0)
		, sample_rate_(stream&& stream->codecpar ? stream->codecpar->sample_rate : 0)
		, time_base_(stream ? stream->time_base : av_make_q(0, 1))
		, seek_pts_(TimeToPts(seek_time, time_base_))
		, stream_(stream)
		, is_eof_(false)
		, acceleration_(acceleration)
		, hw_device_index_(hw_device_index)
		, media_type_(codec ? codec->type : AVMediaType::AVMEDIA_TYPE_UNKNOWN)
		, hw_device_ctx_(NULL, [](AVBufferRef* p) {})
	{

	}

	unique_ptr<AVCodecContext> CreateCodecContext()
	{
		unique_ptr<AVCodecContext> ctx = unique_ptr<AVCodecContext>(codec_ ? avcodec_alloc_context3(codec_) : NULL,
			[](AVCodecContext *c) { if (c) avcodec_free_context(&c); });
		if (!ctx || !stream_ || !codec_)
			THROW_EXCEPTION("Decoder: codec context not created");
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_to_context(ctx.get(), stream_->codecpar));
		if (acceleration_ != Core::HwAccel::none && codec_ && (codec_->id == AV_CODEC_ID_H264 || codec_->id == AV_CODEC_ID_HEVC))
		{
			AVHWDeviceType device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
			AVPixelFormat hw_pix_format = AV_PIX_FMT_NONE;
			switch (acceleration_)
			{
			case Core::HwAccel::cuvid:
				device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA;
				break;
			}
			const AVCodecHWConfig *config = NULL;
			for (int i = 0;; i++) {
				config = avcodec_get_hw_config(codec_, i);
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
				ctx->get_format = GetHwPixelFormat;
			AVBufferRef *weak_hw_device_ctx = NULL;
			if (FF_SUCCESS(av_hwdevice_ctx_create(&weak_hw_device_ctx, device_type, hw_device_index_.c_str(), NULL, 0)))
			{
				ctx->hw_device_ctx = av_buffer_ref(weak_hw_device_ctx);
				hw_device_ctx_ = FFmpeg::unique_ptr<AVBufferRef>(weak_hw_device_ctx, [](AVBufferRef* p) { av_buffer_unref(&p); });
			}
		}

		av_opt_set_int(ctx.get(), "refcounted_frames", 1, 0);
		av_opt_set_int(ctx.get(), "threads", 4, 0);
		THROW_ON_FFMPEG_ERROR(avcodec_open2(ctx.get(), codec_, NULL));
		flush_packet_received_ = false;
		flush_packet_sent_ = false;
		is_eof_ = false;
		DebugPrintLine(Common::DebugSeverity::debug, "Decoder created");
		return ctx;
	}

	void Push(const std::shared_ptr<AVPacket> &packet)
	{
		assert(!packet || packet->stream_index == stream_index_);
		std::lock_guard<std::mutex> lock(mutex_);
		packet_queue_.push_back(packet);
#ifdef DEBUG
		if (packet)
		{
			if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				DebugPrintLine(Common::DebugSeverity::trace, "Queued video packet to decoder:  " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000));
			if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
				DebugPrintLine(Common::DebugSeverity::trace, "Queued audio packet to decoder:  " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000));
		}
		else
		{
			if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				DebugPrintLine(Common::DebugSeverity::debug, "Queued flush packet to video decoder");
			if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
				DebugPrintLine(Common::DebugSeverity::debug, "Queued flush packet to audio decoder");
		}
#endif 
	}

	void PushNextPacket()
	{
		if (packet_queue_.empty())
			return;
		auto packet = packet_queue_.front();
		if (!packet)
			flush_packet_sent_ = true;
		if (flush_packet_sent_ && packet)
			ctx_ = CreateCodecContext();
#ifdef DEBUG
		if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
			if (packet)
				DebugPrintLine(Common::DebugSeverity::trace, "Pushed video packet to video decoder: " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000));
			else
				DebugPrintLine(Common::DebugSeverity::debug, "Pushed flush packet to video decoder");
#endif
		int ret = avcodec_send_packet(ctx_.get(), packet.get());
		switch (ret)
		{
		case 0:
			packet_queue_.pop_front();
			break;
		case AVERROR(EAGAIN):
			DebugPrintLine(Common::DebugSeverity::debug, "PushNextPacket: error EAGAIN");
			break;
		case AVERROR_EOF:
			THROW_EXCEPTION("Decoder: packet pushed after flush");
			break;
		default:
			break;
		}
	}

	std::shared_ptr<AVFrame> Pull()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		PushNextPacket();
		auto frame = AllocFrame();
		auto ret = avcodec_receive_frame(ctx_.get(), frame.get());
		switch (ret)
		{
		case 0:
			if (frame->pts == AV_NOPTS_VALUE)
				frame->pts = frame->best_effort_timestamp;
			if (frame->pts >= seek_pts_ || frame->pts + frame->duration > seek_pts_)
			{
				if (hw_device_ctx_)
				{
					auto sw_frame = AllocFrame();
					THROW_ON_FFMPEG_ERROR(av_hwframe_transfer_data(sw_frame.get(), frame.get(), 0));
					THROW_ON_FFMPEG_ERROR(av_frame_copy_props(sw_frame.get(), frame.get()));
					return sw_frame;
				}
#ifdef DEBUG
				if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
					DebugPrintLine(Common::DebugSeverity::trace, "Pulled video frame from decoder: " + std::to_string(PtsToTime(frame->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(frame->duration, time_base_) / 1000) + ", type: " + av_get_picture_type_char(frame->pict_type));
				if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
					DebugPrintLine(Common::DebugSeverity::trace, "Pulled audio frame from decoder: " + std::to_string(PtsToTime(frame->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(frame->duration, time_base_) / 1000));
#endif 
				return frame;
			}
			return nullptr;
		case AVERROR_EOF:
			is_eof_ = true;
			break;
		case AVERROR(EAGAIN):
			break;
		default:
			THROW_ON_FFMPEG_ERROR(ret);
		}
		return nullptr;
	}

	void Flush()
	{
		Push(nullptr);
		flush_packet_received_ = true;
		DebugPrintLine(Common::DebugSeverity::info, "Decoder flushed");
	}

	void Seek(const std::int64_t seek_time)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		avcodec_flush_buffers(ctx_.get());
		packet_queue_.clear();
		seek_pts_ = TimeToPts(seek_time, time_base_);
	}

};

Decoder::Decoder(const AVCodec *codec, AVStream * const stream, std::int64_t seek_time, Core::HwAccel acceleration, const std::string &hw_device_index)
	: impl_(std::make_unique<implementation>(codec, stream, seek_time, acceleration, hw_device_index))
{ }

Decoder::Decoder(const AVCodec *codec, AVStream * const stream, std::int64_t seek_time)
	: Decoder(codec, stream, seek_time, Core::HwAccel::none, "")
{ }

Decoder::~Decoder() { }

void Decoder::Push(const std::shared_ptr<AVPacket> &packet) { impl_->Push(packet); }

std::shared_ptr<AVFrame> Decoder::Pull() { return impl_->Pull(); }

bool Decoder::IsFlushed() const { return impl_->flush_packet_received_; }

void Decoder::Flush() { impl_->Flush(); }

void Decoder::Seek(const std::int64_t seek_time) { impl_->Seek(seek_time); }

bool Decoder::IsEof() const { return impl_->is_eof_; }

int Decoder::AudioChannelsCount() const { return impl_->channels_count_; }

int Decoder::AudioSampleRate() const { return impl_->sample_rate_; }

int Decoder::StreamIndex() const { return impl_->stream_index_; }

AVChannelLayout * Decoder::AudioChannelLayout() const { return impl_->ctx_ ? &impl_->ctx_->ch_layout : nullptr; }

AVSampleFormat Decoder::AudioSampleFormat() const { return impl_->ctx_ ? impl_->ctx_->sample_fmt : AVSampleFormat::AV_SAMPLE_FMT_NONE; }

AVMediaType Decoder::MediaType() const { return impl_->media_type_; }

const AVRational & Decoder::TimeBase() const { return impl_->time_base_; }

const AVRational & Decoder::FrameRate() const { return impl_->stream_->r_frame_rate; }


}}