#pragma once

namespace TVPlayR {

#if (_MANAGED == 1)
	public
#endif
	enum class TimecodeOverlaySource {
		None,
		Timecode,
		TimeFromBegin,
		TimeToEnd,
		WallTime
	};
}