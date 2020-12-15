#pragma once
#include "../Common/Exceptions.h"

#define FF(value) value >= 0

namespace TVPlayR {
	namespace FFmpeg {

typedef std::shared_ptr<AVFrame> AVFramePtr;
typedef std::shared_ptr<AVPacket> AVPacketPtr;
typedef std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext *)>> AVCodecContextPtr;
typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext *)>> AVFormatCtxPtr;
typedef std::unique_ptr<SwsContext, std::function<void(SwsContext *)>> SwsContextPtr;
typedef std::unique_ptr<AVFilterGraph, std::function<void(AVFilterGraph *)>> AVFilterGraphPtr;
typedef std::unique_ptr<AVBufferRef, std::function<void(AVBufferRef *)>> AVBufferRefPtr;
typedef std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo *)>> AVAudioFifoPtr;



inline AVPacketPtr AllocPacket()
{
	return std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket* p) { av_packet_free(&p); });
}

inline AVFramePtr AllocFrame()
{
	return std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* ptr) { av_frame_free(&ptr); });
}

inline AVFormatCtxPtr GetFormatContext(const std::string& fileName)
{
	AVFormatContext * weak_ctx = NULL;
	if (avformat_alloc_output_context2(&weak_ctx, NULL, NULL, fileName.c_str()) < 0)
		return __nullptr;
	return AVFormatCtxPtr(weak_ctx, [](AVFormatContext * ctx)
	{
		avformat_free_context(ctx);
	});
}

inline void dump_filter(const std::string& filter_str, const AVFilterGraphPtr& graph)
{
	OutputDebugStringA("\nFiltr: ");
	OutputDebugStringA(filter_str.c_str());
	OutputDebugStringA("\n");
	char* filter_dump = avfilter_graph_dump(graph.get(), NULL);
	OutputDebugStringA(filter_dump);
	av_free(filter_dump);
}

}


#define ERROR_STRING_LENGTH 128

#define THROW_ON_FFMPEG_ERROR(error_code) \
if (error_code < 0) \
	{\
	char error_str[ERROR_STRING_LENGTH];\
	av_make_error_string(error_str, ERROR_STRING_LENGTH, error_code);\
	std::string exception_message = std::string(error_str) + std::string(" in ") + std::string(__FUNCTION__) + std::string(" at ") + std::string(__FILE__) + std::string(" in line ") + std::to_string(__LINE__);\
	throw TVPlayR::Common::TVPlayRException(exception_message);\
	}

}