#pragma once

namespace TVPlayR {
	enum class TimecodeOutputSource;
	namespace Core {
		struct FrameTimeInfo;
		std::int64_t TimecodeFromFameTimeInfo(const FrameTimeInfo &frame_time_info, TimecodeOutputSource timecode_source);
	}
}

