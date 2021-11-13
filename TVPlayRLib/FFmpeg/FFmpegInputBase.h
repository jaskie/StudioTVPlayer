#pragma once

#include "InputFormat.h"
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	enum class FieldOrder;

	namespace Core {
		enum class HwAccel;
		class StreamInfo;
	}
	namespace FFmpeg {
		class Decoder;
		
struct FFmpegInputBase : Common::NonCopyable
{
protected:
	FFmpegInputBase(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	const std::string file_name_;
	const Core::HwAccel acceleration_;
	const std::string hw_device_;
	FFmpeg::InputFormat input_;
	const bool is_stream_;
	std::unique_ptr<FFmpeg::Decoder> video_decoder_;
public:
	void InitializeVideoDecoder();
	bool IsStream() const;
	int StreamCount() const;
	const Core::StreamInfo& GetStreamInfo(int index) const;
	std::int64_t GetAudioDuration();
	std::int64_t GetVideoStart() const;
	std::int64_t GetVideoDuration() const;
	AVRational GetTimeBase() const;
	AVRational GetFrameRate() const;
	int GetWidth() const;
	int GetHeight() const;
	TVPlayR::FieldOrder GetFieldOrder() const;
	bool HaveAlphaChannel() const;
	int GetAudioChannelCount() const;
};

}}
