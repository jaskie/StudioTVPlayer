#pragma once

namespace TVPlayR {
	namespace Core
	{
		class VideoFormat;
		enum class VideoFormatType;
	}
	namespace Common 
	{
		template <class T> class Rational;
	}

	enum class PixelFormat;
	enum class DecklinkTimecodeSource;

	namespace Decklink {

		BMDPixelFormat BMDPixelFormatFromPixelFormat(TVPlayR::PixelFormat format);

		BMDDisplayMode GetDecklinkDisplayMode(Core::VideoFormatType fmt);

		Core::VideoFormatType BMDDisplayModeToVideoFormatType(BMDDisplayMode displayMode, bool isWide);

		std::shared_ptr<AVFrame> AVFrameFromDecklinkVideo(IDeckLinkVideoInputFrame* decklink_frame, TVPlayR::DecklinkTimecodeSource timecode_source, const Core::VideoFormat& format, BMDTimeScale time_scale);
		
		std::shared_ptr<AVFrame> AVFrameFromDecklinkAudio(IDeckLinkAudioInputPacket* audio_packet, int channels, AVSampleFormat sample_format, BMDTimeScale sample_rate);

		std::int64_t TimeFromDeclinkTimecode(IDeckLinkVideoInputFrame* decklink_frame, TVPlayR::DecklinkTimecodeSource timecode_source, const Common::Rational<int>& frame_rate);
	}
}
