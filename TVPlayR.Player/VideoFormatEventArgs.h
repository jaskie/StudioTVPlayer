#pragma once
#include "VideoFormat.h"

namespace TVPlayR {

	public ref class VideoFormatEventArgs sealed : EventArgs {
	private:
		initonly VideoFormat^ format_;
	public:
		VideoFormatEventArgs(VideoFormat^ format) {
			format_ = format;
		}
		property VideoFormat^ Format {
			VideoFormat^ get() { return format_; }
		}
	};
}