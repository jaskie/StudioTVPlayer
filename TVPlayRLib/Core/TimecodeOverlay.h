#pragma once
#pragma comment( lib, "gdiplus.lib" )
#include "OverlayBase.h"
namespace TVPlayR {
	namespace FFmpeg {
		class AVSync;
	}
	namespace Core {
		enum class VideoFormatType;

class TimecodeOverlay :  public OverlayBase
{
public:
	TimecodeOverlay(const VideoFormatType video_format, bool no_passthrough_video);
	~TimecodeOverlay();
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
