#pragma once
#include "../Core/HwAccel.h"
#include "../Core/StreamInfo.h"
#include "../Common/NonCopyable.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {

class InputFormat final : public Common::DebugTarget, Common::NonCopyable
{
private:
	std::unique_ptr<AVFormatContext, void(*)(AVFormatContext*)> format_context_;
	std::vector<Core::StreamInfo> streams_;
	const std::string file_name_;
	bool is_eof_ = false;
	bool is_stream_data_loaded_ = false;
public:
	InputFormat(const std::string& fileName);
	bool LoadStreamData();
	std::shared_ptr<AVPacket> PullPacket();
	bool CanSeek() const;
	bool Seek(std::int64_t time);
	inline operator bool() { return !!format_context_; }
	inline bool IsEof() const { return is_eof_; }
	inline bool IsStreamDataLoaded() const { return is_stream_data_loaded_; }
	int GetTotalAudioChannelCount() const;
	std::int64_t ReadStartTimecode() const;
	const std::vector<Core::StreamInfo>& GetStreams() const { return streams_; };
	const Core::StreamInfo* GetVideoStream() const;
	bool IsValid() const;
};

}}