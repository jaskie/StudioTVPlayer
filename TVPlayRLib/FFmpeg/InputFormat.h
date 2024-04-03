#pragma once
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace Core {
		struct StreamInfo;
		enum class HwAccel;
	}

	namespace FFmpeg {

class InputFormat final : public Common::DebugTarget, Common::NonCopyable
{
private:
	unique_ptr<AVFormatContext> format_context_;
	std::vector<Core::StreamInfo> streams_;
	const std::string file_name_;
	bool is_eof_ = false;
	bool is_stream_data_loaded_ = false;
	std::mutex seek_mutex_;
public:
	InputFormat(const std::string &fileName);
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
	const Core::StreamInfo * GetVideoStream() const;
	bool IsValid() const;
};

}}