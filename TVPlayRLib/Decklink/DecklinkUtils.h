#pragma once
#include "../Core/PixelFormat.h"
#include "../Core/VideoFormat.h"
#include "DeckLinkAPI_h.h"

namespace TVPlayR {
	namespace Decklink {
		enum class DecklinkTimecodeSource;

		typedef void(*FORMAT_CALLBACK)(Core::VideoFormatType new_format);

		BMDPixelFormat BMDPixelFormatFromVideoFormat(const Core::PixelFormat& format);

		BMDDisplayMode GetDecklinkDisplayMode(Core::VideoFormatType fmt);

		Core::VideoFormatType BMDDisplayModeToVideoFormatType(BMDDisplayMode displayMode, bool isWide);

		std::shared_ptr<AVFrame> AVFrameFromDecklink(IDeckLinkVideoInputFrame* decklink_frame, DecklinkTimecodeSource timecode_source, const Core::VideoFormat& format, BMDTimeScale time_scale);
	}
}
