#pragma once
using namespace System;

namespace TVPlayR {

	public ref class TimeEventArgs sealed : EventArgs {
	private:
		initonly TimeSpan timecode_;
		initonly TimeSpan time_from_begin_;
		initonly TimeSpan time_to_end_;
	public:
		TimeEventArgs(TimeSpan timecode, TimeSpan time_from_begin, TimeSpan time_to_end) 
		{
			timecode_ = timecode;
			time_from_begin_ = time_from_begin;
			time_to_end_ = time_to_end;
		}
		property TimeSpan Timecode { TimeSpan get() { return timecode_; } }
		property TimeSpan TimeFromBegin { TimeSpan get() { return time_from_begin_; } }
		property TimeSpan TimeToEnd { TimeSpan get() { return time_to_end_; } }
	};
}

