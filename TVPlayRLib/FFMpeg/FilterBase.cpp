#include "../pch.h"
#include "FilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {
FilterBase::FilterBase()
	: graph_(nullptr, [](AVFilterGraph* g) { avfilter_graph_free(&g); })
{
}

int64_t FilterBase::TimeFromTs(int64_t ts) const
{
	return PtsToTime(ts, OutputTimeBase());
}

}}