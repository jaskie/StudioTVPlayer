#pragma once
#pragma comment( lib, "gdiplus.lib" )
#include "Overlay.h"
namespace TVPlayR {
	namespace FFmpeg {
		class AVSync;
	}
	namespace Core {

class TimecodeOverlay :  public Overlay
{
public:
	TimecodeOverlay();
	~TimecodeOverlay();
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
