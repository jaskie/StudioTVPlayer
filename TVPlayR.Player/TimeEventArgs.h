#pragma once
using namespace System;

namespace TVPlayR {

	public ref class TimeEventArgs : EventArgs {
	private:
		initonly TimeSpan time_;
	public:
		TimeEventArgs(TimeSpan time) {
			time_ = time;
		}
		property TimeSpan Time {
			TimeSpan get() { return time_; }
		}
	};

}

