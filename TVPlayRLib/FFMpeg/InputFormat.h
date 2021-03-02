#pragma once
#include "Utils.h"
#include "../Core/HwAccel.h"
#include "../Core/StreamInfo.h"

namespace TVPlayR {
	namespace FFmpeg {

class InputFormat
{
private:
	AVFormatCtxPtr format_context_;
	std::vector<Core::StreamInfo> streams_;
	bool is_eof_ = false;
	bool is_stream_data_loaded_ = false;
public:
	InputFormat(const std::string& fileName);
	bool LoadStreamData();
	std::shared_ptr<AVPacket> PullPacket();
	bool CanSeek() const;
	bool Seek(int64_t time);
	inline operator bool() { return !!format_context_; }
	inline bool IsEof() const { return is_eof_; }
	inline bool IsStreamDataLoaded() const { return is_stream_data_loaded_; }
	int GetTotalAudioChannelCount() const;
	std::vector<Core::StreamInfo>& GetStreams();
	const Core::StreamInfo* GetVideoStream() const;
};

}}