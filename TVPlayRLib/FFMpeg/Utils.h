#pragma once
#include "../Common/Exceptions.h"

#define FF(value) (value >= 0)

namespace TVPlayR {
	namespace Core {
		class VideoFormat;
		enum class PixelFormat;
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
	throw TVPlayR::Common::TVPlayRException(exception_message);\
	}

typedef std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext *)>> AVCodecContextPtr;
typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext *)>> AVFormatCtxPtr;
typedef std::unique_ptr<SwsContext, std::function<void(SwsContext *)>> SwsContextPtr;
typedef std::unique_ptr<AVFilterGraph, std::function<void(AVFilterGraph *)>> AVFilterGraphPtr;
typedef std::unique_ptr<AVBufferRef, std::function<void(AVBufferRef *)>> AVBufferRefPtr;
typedef std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo *)>> AVAudioFifoPtr;


inline std::shared_ptr<AVPacket> AllocPacket()
{
	return std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket* p) { av_packet_free(&p); });
}

inline std::shared_ptr<AVFrame> AllocFrame()
{
	return std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* ptr) { av_frame_free(&ptr); });
}

inline int64_t PtsToTime(int64_t pts, AVRational time_base)
{
	if (pts == AV_NOPTS_VALUE)
		return pts;
	return av_rescale(pts * AV_TIME_BASE, time_base.num, time_base.den);
}

inline int64_t TimeToPts(int64_t time, AVRational time_base)
{
	if (time == AV_NOPTS_VALUE || time == 0)
		return time;
	return av_rescale(time, time_base.den, static_cast<int64_t>(time_base.num) * AV_TIME_BASE);
}

std::shared_ptr<AVFrame> CreateEmptyVideoFrame(const Core::VideoFormat& format, Core::PixelFormat pix_fmt);

std::shared_ptr<AVFrame> CreateSilentAudioFrame(int samples_count, int num_channels);

void dump_filter(const std::string& filter_str, const AVFilterGraphPtr& graph);

}}