#pragma once
#include "../PixelFormat.h"
#include "../Core/VideoFormat.h"
#include "DeckLinkAPI_h.h"

namespace TVPlayR {
	enum class DecklinkTimecodeSource;

	namespace Decklink {

		typedef void(*FORMAT_CALLBACK)(Core::VideoFormatType new_format);

		BMDPixelFormat BMDPixelFormatFromVideoFormat(TVPlayR::PixelFormat format);

		BMDDisplayMode GetDecklinkDisplayMode(Core::VideoFormatType fmt);

		Core::VideoFormatType BMDDisplayModeToVideoFormatType(BMDDisplayMode displayMode, bool isWide);

		std::shared_ptr<AVFrame> AVFrameFromDecklinkVideo(IDeckLinkVideoInputFrame* decklink_frame, TVPlayR::DecklinkTimecodeSource timecode_source, const Core::VideoFormat& format, BMDTimeScale time_scale);
		
		std::shared_ptr<AVFrame> AVFrameFromDecklinkAudio(IDeckLinkAudioInputPacket* audio_packet, int channels, AVSampleFormat sample_format, BMDTimeScale sample_rate);

		int64_t TimeFromDeclinkTimecode(IDeckLinkVideoInputFrame* decklink_frame, TVPlayR::DecklinkTimecodeSource timecode_source, const Common::Rational<int>& frame_rate);
	}
}
