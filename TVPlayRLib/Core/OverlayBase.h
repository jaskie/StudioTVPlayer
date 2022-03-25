#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace FFmpeg {
		struct AVSync;
	}

	namespace Core {

class OverlayBase : public Common::NonCopyable
{
public:
	virtual FFmpeg::AVSync Transform(FFmpeg::AVSync& sync) = 0;
};

}}