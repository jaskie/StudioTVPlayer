#pragma once
#pragma comment( lib, "gdiplus.lib" )
#include "OverlayBase.h"

namespace TVPlayR {
		enum class PixelFormat;	
		enum class TimecodeOutputSource;

	namespace Core {
		enum class VideoFormatType;

class TimecodeOverlay final :  public OverlayBase
{
public:
	TimecodeOverlay(const TimecodeOutputSource source, const VideoFormatType video_format, TVPlayR::PixelFormat output_pixel_format);
	virtual ~TimecodeOverlay();
	Core::AVSync Transform(Core::AVSync& sync) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
