#pragma once
#pragma comment( lib, "gdiplus.lib" )
#include "OverlayBase.h"
namespace TVPlayR {
		enum class PixelFormat;	
	
	namespace FFmpeg {
		class AVSync;
	}
	namespace Core {
		enum class VideoFormatType;

class TimecodeOverlay final :  public OverlayBase
{
public:
	TimecodeOverlay(const VideoFormatType video_format, TVPlayR::PixelFormat output_pixel_format);
	~TimecodeOverlay();
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
