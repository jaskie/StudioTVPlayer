#pragma once

namespace TVPlayR {

#if (_MANAGED == 1)
public
#endif
enum class PixelFormat {
	yuv422,
	bgra,
	rgb10
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
	case PixelFormat::rgb10:
		return AV_PIX_FMT_X2RGB10LE;
	default:
		THROW_EXCEPTION("Invalid pixel format: " + std::to_string(static_cast<int>(format)));
	}
}
#endif

}
