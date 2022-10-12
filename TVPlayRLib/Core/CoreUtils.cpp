#include "../pch.h"
#include "CoreUtils.h"
#include "../TimecodeOutputSource.h"
#include "FrameTimeInfo.h"

namespace TVPlayR {
	namespace Core {
		int64_t TimecodeFromFameTimeInfo(FrameTimeInfo& frame_time_info, TimecodeOutputSource timecode_source)
		{
			switch (timecode_source)
			{
			case TimecodeOutputSource::Timecode:
				return frame_time_info.Timecode;
			case TimecodeOutputSource::TimeFromBegin:
				return frame_time_info.TimeFromBegin;
			case TimecodeOutputSource::TimeToEnd:
				return frame_time_info.TimeToEnd;
				break;
			case TimecodeOutputSource::WallTime:
				SYSTEMTIME system_time;
				::GetLocalTime(&system_time);
				return((static_cast<int64_t>(system_time.wHour) * 60 + system_time.wMinute) * 60 + system_time.wSecond) * AV_TIME_BASE + system_time.wMilliseconds * 1000;
			}
		}
	}
}
