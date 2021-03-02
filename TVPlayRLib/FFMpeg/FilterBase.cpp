#include "../pch.h"
#include "FilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {

int64_t FilterBase::TimeFromTs(int64_t ts) const
{
	return PtsToTime(ts, OutputTimeBase());
}

}}