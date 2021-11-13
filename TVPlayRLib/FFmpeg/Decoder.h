#pragma once
#include "../Core/HwAccel.h"
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace FFmpeg {

class Decoder final : private Common::NonCopyable
{
public:
	Decoder(const AVCodec* codec, AVStream * const stream, std::int64_t seek_time, Core::HwAccel acceleration, const std::string& device_index);
	Decoder(const AVCodec* codec, AVStream * const stream, std::int64_t seek_time);
	~Decoder();
	void Push(const std::shared_ptr<AVPacket>& packet);
	std::shared_ptr<AVFrame> Pull();
	const AVRational& TimeBase() const;
	bool IsFlushed() const;
	void Flush();
	void Seek(const std::int64_t seek_time);
	bool IsEof() const;
	int AudioChannelsCount() const;
	int AudioSampleRate() const;
	std::uint64_t AudioChannelLayout() const;
	AVSampleFormat AudioSampleFormat() const;
	AVMediaType MediaType() const;
	const AVRational& FrameRate() const;
	int StreamIndex() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}