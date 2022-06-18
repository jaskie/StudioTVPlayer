#pragma once

#if (_MANAGED == 1)
using System::ComponentModel::DescriptionAttribute;
#endif

namespace TVPlayR {

#if (_MANAGED == 1)
	public
#endif
	enum class TimecodeOverlaySource {
		None,
		Timecode,
#if (_MANAGED == 1)
		[Description("Time from begin")]
#endif
		TimeFromBegin,
#if (_MANAGED == 1)
		[Description("Time to end")]
#endif
		TimeToEnd,
#if (_MANAGED == 1)
		[Description("Wall clock time")]
#endif
		WallTime
	};
}