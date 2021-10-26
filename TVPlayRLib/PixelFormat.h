#pragma once
#include "Common/Exceptions.h"

namespace TVPlayR {

#if (_MANAGED == 1)
		public
#endif
enum class PixelFormat {
	yuv422,
	bgra
};

#if (_MANAGED != 1)
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
#endif

}