#include "../pch.h"
#include "FilterBase.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
FilterBase::FilterBase()
	: graph_(nullptr, [](AVFilterGraph* g) { avfilter_graph_free(&g); })
{
}

std::int64_t FilterBase::TimeFromTs(std::int64_t ts) const
{
	return PtsToTime(ts, OutputTimeBase());
}

}}