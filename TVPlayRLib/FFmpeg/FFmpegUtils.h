#pragma once

#define FF(value) (value >= 0)

namespace TVPlayR {
		enum class PixelFormat;
	namespace Core {
		class VideoFormat;
	}
	namespace FFmpeg {

#define ERROR_STRING_LENGTH 128

#define THROW_ON_FFMPEG_ERROR(error_code) \
if (error_code < 0) \
	{\
	char error_str[ERROR_STRING_LENGTH];\
	av_make_error_string(error_str, ERROR_STRING_LENGTH, error_code);\
	std::string exception_message = std::string(error_str) + std::string(" in ") + std::string(__FUNCTION__) + std::string(" at ") + std::string(__FILE__) + std::string(" in line ") + std::to_string(__LINE__);\
	OutputDebugStringA((exception_message + "\n").c_str());\
	throw TVPlayR::Common::TVPlayRException(exception_message.c_str());\
	}

template<class T> using unique_ptr = std::unique_ptr<T, void(*)(T*)>; // unique pointer type for FFmpeg wrappers

std::shared_ptr<AVPacket> AllocPacket();

std::shared_ptr<AVFrame> AllocFrame();

std::shared_ptr<AVFrame> CloneFrame(const std::shared_ptr<AVFrame>& source);

std::shared_ptr<AVFrame> CopyFrame(const std::shared_ptr<AVFrame>& source);

inline std::int64_t PtsToTime(std::int64_t pts, const AVRational time_base)
{
	if (pts == AV_NOPTS_VALUE)
		return pts;
	return av_rescale(pts * AV_TIME_BASE, time_base.num, time_base.den);
}

inline std::int64_t TimeToPts(std::int64_t time, const AVRational time_base)
{
	if (time == AV_NOPTS_VALUE || time == 0)
		return time;
	return av_rescale(time, time_base.den, static_cast<std::int64_t>(time_base.num) * AV_TIME_BASE);
}

std::shared_ptr<AVFrame> CreateEmptyVideoFrame(const Core::VideoFormat& format, TVPlayR::PixelFormat pix_fmt);

std::shared_ptr<AVFrame> CreateSilentAudioFrame(int samples_count, int num_channels, AVSampleFormat format);

void DumpFilter(const std::string& filter_str, AVFilterGraph* graph);

bool HaveAlphaChannel(AVPixelFormat format);

AVDictionary* ReadOptions(const std::string& params);

}}