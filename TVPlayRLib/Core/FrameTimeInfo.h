#pragma once
namespace TVPlayR {
	namespace Core {
		struct FrameTimeInfo
		{
			std::int64_t Timecode = AV_NOPTS_VALUE;
			std::int64_t TimeFromBegin = AV_NOPTS_VALUE;
			std::int64_t TimeToEnd = AV_NOPTS_VALUE;
		};
	}
}