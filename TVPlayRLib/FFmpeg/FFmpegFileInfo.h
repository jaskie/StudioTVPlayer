#pragma once
#include "../Core/HwAccel.h"

namespace TVPlayR {
	enum class FieldOrder;
	
	namespace Core {
		class AudioChannelMapEntry;
		class StreamInfo;
	}

	namespace FFmpeg {

class FFmpegFileInfo final
{
public:
	FFmpegFileInfo(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device);
	~FFmpegFileInfo();
	std::shared_ptr<AVFrame> GetFrameAt(std::int64_t time);
	AVRational GetTimeBase() const;
	AVRational GetFrameRate() const;
	std::int64_t GetAudioDuration() const;
	std::int64_t GetVideoStart() const;
	std::int64_t GetVideoDuration() const;
	int GetWidth() const;
	int GetHeight() const;
	TVPlayR::FieldOrder GetFieldOrder() const;
	int GetAudioChannelCount() const;
	bool HaveAlphaChannel()const;
	int StreamCount() const;
	const Core::StreamInfo& GetStreamInfo(int index) const;
	bool IsStream() const;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}