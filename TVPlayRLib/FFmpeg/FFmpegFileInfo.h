#pragma once
#include "../Core/HwAccel.h"

namespace TVPlayR {
	namespace Core {
		class AudioChannelMapEntry;
		class StreamInfo;
		enum class FieldOrder;
	}
		namespace FFmpeg {

class FFmpegFileInfo
{
public:
	FFmpegFileInfo(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	~FFmpegFileInfo();
	std::shared_ptr<AVFrame> GetFrameAt(int64_t time);
	AVRational GetTimeBase() const;
	AVRational GetFrameRate() const;
	int64_t GetAudioDuration() const;
	int64_t GetVideoStart() const;
	int64_t GetVideoDuration() const;
	int GetWidth() const;
	int GetHeight() const;
	Core::FieldOrder GetFieldOrder();
	int GetAudioChannelCount();
	bool HaveAlphaChannel()const;
	int StreamCount() const;
	const Core::StreamInfo& GetStreamInfo(int index) const;
	bool IsStream() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}