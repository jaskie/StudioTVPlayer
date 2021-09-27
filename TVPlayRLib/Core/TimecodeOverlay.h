#pragma once
#pragma comment( lib, "gdiplus.lib" )
#include "OverlayBase.h"
namespace TVPlayR {
	namespace FFmpeg {
		class AVSync;
	}
	namespace Core {
		enum class VideoFormatType;
		enum class PixelFormat;

class TimecodeOverlay :  public OverlayBase
{
public:
	TimecodeOverlay(const VideoFormatType video_format, PixelFormat output_pixel_format);
	~TimecodeOverlay();
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
