#include "../pch.h"
#include "Utils.h"
#include "Decoder.h"
#include "AudioFifo.h"

#undef DEBUG

namespace TVPlayR {
	namespace FFmpeg {

AVPixelFormat GetHwPixelFormat(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
	const AVPixelFormat* p;
	for (p = pix_fmts; *p != -1; p++)
	{
		if (*p == AV_PIX_FMT_CUDA)
			return *p;
	}
	return AV_PIX_FMT_NONE;
}

struct Decoder::implementation
{
	const int64_t start_ts_;
	const Core::HwAccel acceleration_;
	const std::string hw_device_index_;
	std::unique_ptr<AVBufferRef, void(*)(AVBufferRef*)> hw_device_ctx_;
	bool is_eof_ = false;
	bool is_flushed_ = false;
	const int stream_index_;
	const int channels_count_;
	const int sample_rate_;
	const std::unique_ptr<AVCodecContext, void(*)(AVCodecContext*)>  ctx_;
	std::deque<std::shared_ptr<AVPacket>> packet_queue_;
	const AVRational time_base_;
	AVStream* const stream_;
	const AVMediaType media_type_;
	int64_t seek_pts_;
	const int64_t duration_;

	implementation(const AVCodec* codec, AVStream* const stream, int64_t seek_time, Core::HwAccel acceleration, const std::string& hw_device_index)
		: ctx_(codec ? avcodec_alloc_context3(codec) : NULL, [](AVCodecContext* c) { if (c)	avcodec_free_context(&c); })
		, start_ts_(stream ? stream->start_time : 0LL)
		, duration_(stream ? stream->duration: 0LL)
		, stream_index_(stream ? stream->index : 0)
		, channels_count_(stream && stream->codecpar ? stream->codecpar->channels : 0)
		, sample_rate_(stream && stream->codecpar ? stream->codecpar->sample_rate : 0)
		, time_base_(stream ? stream->time_base : av_make_q(0, 1))
		, seek_pts_(TimeToPts(seek_time, time_base_))
		, stream_(stream)
		, is_eof_(false)
		, acceleration_(acceleration)
		, hw_device_index_(hw_device_index)
		, media_type_(codec ? codec->type : AVMediaType::AVMEDIA_TYPE_UNKNOWN)
		, hw_device_ctx_(NULL, [](AVBufferRef* p) { av_buffer_unref(&p); })
	{
		if (!ctx_ || !stream || !codec)
			return;
		THROW_ON_FFMPEG_ERROR(avcodec_parameters_to_context(ctx_.get(), stream->codecpar));
		if (acceleration != Core::HwAccel::none && codec && (codec->id == AV_CODEC_ID_H264 || codec->id == AV_CODEC_ID_HEVC))
		{
			AVHWDeviceType device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
			AVPixelFormat hw_pix_format = AV_PIX_FMT_NONE;
			switch (acceleration)
			{
			case Core::HwAccel::cuvid:
				device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA;
				break;
			}
			const AVCodecHWConfig* config = NULL;
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
				ctx_->get_format = GetHwPixelFormat;
			AVBufferRef* weak_hw_device_ctx = NULL;
			if (FF(av_hwdevice_ctx_create(&weak_hw_device_ctx, device_type, hw_device_index_.c_str(), NULL, 0)))
			{
				hw_device_ctx_.reset(weak_hw_device_ctx);
				ctx_->hw_device_ctx = av_buffer_ref(weak_hw_device_ctx);
			}
		}

		av_opt_set_int(ctx_.get(), "refcounted_frames", 0, 0);
		av_opt_set_int(ctx_.get(), "threads", 4, 0);
		THROW_ON_FFMPEG_ERROR(avcodec_open2(ctx_.get(), codec, NULL));
	}

	void Push(const std::shared_ptr<AVPacket>& packet)
	{
		assert(!packet || packet->stream_index == stream_index_);
#ifdef DEBUG
		if (packet)
		{
			if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				OutputDebugStringA(("Pushed video packet to decoder:  " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000) + "\n").c_str());
			if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
				OutputDebugStringA(("Pushed audio packet to decoder:  " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000) + "\n").c_str());
		}
		else
		{
			if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				OutputDebugStringA("Pushed flush packet to video decoder\n");
			if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
				OutputDebugStringA("Pushed flush packet to audio decoder\n");
		}
#endif 
#ifdef DEBUG
		{
#endif
			packet_queue_.push_back(packet);
#ifdef DEBUG
			if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
				OutputDebugStringA(("Queued video packet: " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000) + "\n").c_str());
		}
#endif
	}

	bool PushNextPacket()
	{
		if (packet_queue_.empty())
			return false;
		auto packet = packet_queue_.front();
#ifdef DEBUG
		if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
			OutputDebugStringA(("Pushed video packet to decoder: " + std::to_string(PtsToTime(packet->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(packet->duration, time_base_) / 1000) + "\n").c_str());
#endif
		int ret = avcodec_send_packet(ctx_.get(), packet.get());
		switch (ret)
		{
		case 0:
			packet_queue_.pop_front();
			return true;
		case AVERROR(EAGAIN):
			break;
		case AVERROR_EOF:
			THROW_EXCEPTION("Packet pushed after flush");
			break;
		default:
			break;
		}
		return false;
	}

	std::shared_ptr<AVFrame> Pull()
	{
		while (!packet_queue_.empty())
		{
			PushNextPacket();
			auto frame = AllocFrame();
			auto ret = avcodec_receive_frame(ctx_.get(), frame.get());
			switch (ret)
			{
			case 0:
				if (frame->pts == AV_NOPTS_VALUE)
					frame->pts = frame->best_effort_timestamp;
				if (frame->pts >= seek_pts_ || frame->pts + frame->pkt_duration > seek_pts_)
				{
					if (hw_device_ctx_)
					{
						auto sw_frame = AllocFrame();
						THROW_ON_FFMPEG_ERROR(av_hwframe_transfer_data(sw_frame.get(), frame.get(), 0));
						THROW_ON_FFMPEG_ERROR(av_frame_copy_props(sw_frame.get(), frame.get()));
						frame = sw_frame;
					}
#ifdef DEBUG
					if (ctx_->codec_type == AVMEDIA_TYPE_VIDEO)
						OutputDebugStringA(("Pulled video frame from decoder: " + std::to_string(PtsToTime(frame->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(frame->pkt_duration, time_base_) / 1000) + ", type: " + av_get_picture_type_char(frame->pict_type) + "\n").c_str());
					if (ctx_->codec_type == AVMEDIA_TYPE_AUDIO)
						OutputDebugStringA(("Pulled audio frame from decoder: " + std::to_string(PtsToTime(frame->pts, time_base_) / 1000) + ", duration: " + std::to_string(PtsToTime(frame->pkt_duration, time_base_) / 1000) + "\n").c_str());
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
		}
		return nullptr;
	}
	
	void Flush()
	{
		if (is_flushed_)
			return;
		is_flushed_ = true; 
		Push(nullptr);
	}

	void Seek(const int64_t seek_time)
	{
		avcodec_flush_buffers(ctx_.get());
		is_eof_ = false;
		is_flushed_ = false;
		packet_queue_.clear();
		seek_pts_ = TimeToPts(seek_time, time_base_);
	}

};

Decoder::Decoder(const AVCodec* codec, AVStream* const stream, int64_t seek_time, Core::HwAccel acceleration, const std::string& hw_device_index)
	: impl_(std::make_unique<implementation>(codec, stream, seek_time, acceleration, hw_device_index))
{ }

Decoder::Decoder(const AVCodec * codec, AVStream * const stream, int64_t seek_time)
	: Decoder(codec, stream, seek_time, Core::HwAccel::none, "")
{ }

Decoder::~Decoder() { }

void Decoder::Push(const std::shared_ptr<AVPacket>& packet) { impl_->Push(packet); }

std::shared_ptr<AVFrame> Decoder::Pull() { return impl_->Pull(); }

void Decoder::Flush() { impl_->Flush(); }

void Decoder::Seek(const int64_t seek_time) { impl_->Seek(seek_time); }

bool Decoder::IsEof() const { return impl_->is_eof_; }

int Decoder::AudioChannelsCount() const { return impl_->channels_count_; }

int Decoder::AudioSampleRate() const { return impl_->sample_rate_; }

int Decoder::StreamIndex() const { return impl_->stream_index_; }

uint64_t Decoder::AudioChannelLayout() const { return impl_->ctx_ ? impl_->ctx_->channel_layout : 0ULL; }

AVSampleFormat Decoder::AudioSampleFormat() const { return impl_->ctx_ ? impl_->ctx_->sample_fmt : AVSampleFormat::AV_SAMPLE_FMT_NONE; }

AVMediaType Decoder::MediaType() const { return impl_->media_type_; }

const AVRational& Decoder::TimeBase() const { return impl_->time_base_; }

const AVRational& Decoder::FrameRate() const { return impl_->stream_->r_frame_rate; }


}}