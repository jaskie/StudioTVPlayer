#pragma once
#include "Utils.h"
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace FFmpeg {

class InputFormat
{
private:
	AVFormatCtxPtr format_context_;
	std::vector<AVStream*> audio_streams_;
	int video_stream_index_ = AVERROR_STREAM_NOT_FOUND;
	AVCodec* audio_codec_ = NULL;
	AVCodec* video_codec_ = NULL;
	bool is_eof_ = false;
public:
	InputFormat(const std::string& fileName);
	int64_t GetVideoDuration() const;
	int64_t GetAudioDuration() const;
	const std::vector<AVStream*>& GetAudioStreams() const;
	AVStream* GetVideoStream() const;
	AVCodec* GetAudioCodec() const;
	AVCodec* GetVideoCodec() const;
	AVPacketPtr PullPacket();
	bool Seek(int64_t time);
	inline operator bool() { return !!format_context_; }
	inline bool IsEof() const { return is_eof_; }
};

}}