#pragma once

#include "../InputFormat.h"

namespace TVPlayR {
	namespace Core {
		enum class HwAccel;
		enum class FieldOrder;
		class StreamInfo;
	}
	namespace FFmpeg {
		class Decoder;
		
		namespace internal {

struct FFmpegInputBase
{
	FFmpegInputBase(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	const std::string file_name_;
	const Core::HwAccel acceleration_;
	const std::string hw_device_;
	FFmpeg::InputFormat input_;
	const bool is_stream_;
	std::unique_ptr<FFmpeg::Decoder> video_decoder_;

	void InitializeVideoDecoder();
	bool IsStream(const std::string& fileName) const;
	int StreamCount() const;
	const Core::StreamInfo& GetStreamInfo(int index) const;
	int64_t GetAudioDuration();
	int64_t GetVideoStart() const;
	int64_t GetVideoDuration() const;
	AVRational GetTimeBase() const;
	AVRational GetFrameRate() const;
	int GetWidth() const;
	int GetHeight() const;
	Core::FieldOrder GetFieldOrder() const;
	bool HaveAlphaChannel() const;
	int GetAudioChannelCount() const;
};

}}}
