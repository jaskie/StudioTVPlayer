#pragma once
#include "../FFMpeg/Utils.h"

namespace TVPlayR {
	namespace Core {

enum class PixelFormat {
	yuv422,
	bgra,
};

static AVPixelFormat PixelFormatToFFmpegFormat(const PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::bgra:
		return AV_PIX_FMT_BGRA;
	case PixelFormat::yuv422:
		return AV_PIX_FMT_UYVY422;
	default:
		THROW_EXCEPTION("invalid pixel format")
	}
}

}}
