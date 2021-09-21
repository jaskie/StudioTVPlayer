#pragma once
#include "../Common/NonCopyable.h"
#include "../FFmpeg/AVSync.h"

namespace TVPlayR {
	namespace Core {

class OverlayBase : public Common::NonCopyable
{
public:
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) = 0;
};

}}