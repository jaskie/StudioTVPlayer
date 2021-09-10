#pragma once
#include "../Core/HwAccel.h"
#include "../Core/StreamInfo.h"

namespace TVPlayR {
	namespace FFmpeg {

class InputFormat
{
private:
	std::unique_ptr<AVFormatContext, void(*)(AVFormatContext*)> format_context_;
	std::vector<Core::StreamInfo> streams_;
	bool is_eof_ = false;
	bool is_stream_data_loaded_ = false;
	int64_t start_timecode_;
	int64_t ReadStartTimecode() const;
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
	int64_t GetStartTimecode() const { return start_timecode_; };
	std::vector<Core::StreamInfo>& GetStreams() { return streams_; };
	const Core::StreamInfo* GetVideoStream() const;
};

}}