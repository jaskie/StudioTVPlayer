#include "../pch.h"
#include "FilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {
FilterBase::FilterBase()
	: graph_(nullptr, [](AVFilterGraph *g) { avfilter_graph_free(&g); })
{
}

}}