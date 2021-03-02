#pragma once
#include "Utils.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace FFmpeg {

class Decoder
{
public:
	Decoder(const AVCodec* codec, AVStream * const stream, int64_t seek_time, Core::HwAccel acceleration, const std::string& device_index);
	Decoder(const AVCodec* codec, AVStream * const stream, int64_t seek_time);
	~Decoder();
	bool Push(const std::shared_ptr<AVPacket>& packet);
	std::shared_ptr<AVFrame> Pull();
	bool Flush();
	void Seek(const int64_t seek_time);
	bool IsEof() const;
	int AudioChannelsCount() const;
	int AudioSampleRate() const;
	uint64_t AudioChannelLayout() const;
	AVSampleFormat AudioSampleFormat() const;
	AVMediaType MediaType() const;
	AVRational TimeBase() const;
	AVRational FrameRate() const;
	int StreamIndex() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}