#include "../pch.h"
#include "FilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {

int64_t FilterBase::TimeFromTs(int64_t ts) const
{
	return av_rescale_q(ts, OutputTimeBase(), av_get_time_base_q());
}

}}