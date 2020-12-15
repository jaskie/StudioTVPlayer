#pragma once
#include "Utils.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace FFmpeg {

class AudioFifo;

class Decoder
{
public:
	Decoder(const AVCodec* codec, AVStream * const stream, Core::HwAccel acceleration, const std::string& device_index);
	Decoder(const AVCodec* codec, AVStream * const stream, const int64_t fifo_duration);
	~Decoder();
	void PushPacket(AVPacketPtr packet);
	AVFramePtr PullVideo();
	AVFramePtr PullAudio();
	void Flush();
	void Seek(const int64_t seek_time);
	int64_t TimeFromTs(int64_t ts) const;
	int64_t TimeMin();
	int64_t TimeMax();
	int64_t FrontFrameEndTime() const;
	bool IsEof() const;
	inline bool IsFlushed() const { return is_flushed_; }
	operator bool() const { return static_cast<bool>(ctx); }
	bool Empty() const;
	const int stream_index;
	const int channels_count;
	const int sample_rate;
	const AVCodecContextPtr ctx;
	const AVRational frame_rate;
	AVStream * const stream;

private:
	const int64_t start_ts;
	const Core::HwAccel acceleration_;
	const std::string hw_device_index_;
	AVBufferRefPtr hw_device_ctx_;
	std::deque<AVFramePtr> frame_buffer_;
	std::unique_ptr<AudioFifo> fifo_;
	bool is_eof_ = false;
	bool is_flushed_ = false;
	int64_t seek_time_ = 0LL;
	void ReadFrames();
	void CopyPushFrame(AVFramePtr& frame);
	bool TryFeedFifo(AVFramePtr frame);
};

}}