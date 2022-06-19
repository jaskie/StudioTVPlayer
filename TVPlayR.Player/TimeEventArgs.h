#pragma once
using namespace System;

namespace TVPlayR {

	public ref class TimeEventArgs sealed : EventArgs {
	private:
		initonly TimeSpan time_from_begin_;
		initonly Nullable<TimeSpan> timecode_;
		initonly Nullable<TimeSpan> time_to_end_;
	public:
		TimeEventArgs(std::int64_t time_from_begin, std::int64_t time_to_end, std::int64_t timecode)
		{
			time_from_begin_ = TimeSpan(time_from_begin * 10);
			if (time_to_end != AV_NOPTS_VALUE)
				time_to_end_ = TimeSpan(time_to_end * 10);
			if (timecode != AV_NOPTS_VALUE)
				timecode_ = TimeSpan(timecode * 10);
		}

		TimeEventArgs(TimeSpan time_from_begin, Nullable<TimeSpan> time_to_end, Nullable<TimeSpan> timecode)
		{
			time_from_begin_ = time_from_begin;
			timecode_ = timecode;
			time_to_end_ = time_to_end;
		}
		property TimeSpan TimeFromBegin { TimeSpan get() { return time_from_begin_; } }
		property Nullable<TimeSpan> Timecode { Nullable<TimeSpan> get() { return timecode_; } }
		property Nullable<TimeSpan> TimeToEnd { Nullable<TimeSpan> get() { return time_to_end_; } }
	};
}

