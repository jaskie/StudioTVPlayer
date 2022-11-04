#ifndef PCH_H
#define PCH_H
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG

#define REWRAP_EXCEPTION(statement) try { statement } catch (std::exception e) { throw gcnew System::Exception(gcnew System::String(e.what())); }

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <numeric>
#include <limits>
#include <Windows.h>
#include <objbase.h>
#include <comdef.h>
#include "Decklink/DeckLinkAPI_h.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/rational.h"
#include "libavutil/mathematics.h"
#include "libavutil/dict.h"
#include "libavutil/opt.h"
#include "libavutil/avutil.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
#include "libavutil/audio_fifo.h"
#include "libswscale/swscale.h"
#include "libavutil/timecode.h"
#include "Processing.NDI.Lib.h"
}
#include "Common/Rational.h"
#include "Common/Debug.h"
#include "Common/NonCopyable.h"
#endif //PCH_H
