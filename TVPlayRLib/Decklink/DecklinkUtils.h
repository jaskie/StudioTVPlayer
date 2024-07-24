#pragma once

namespace TVPlayR {
	
	enum class FieldOrder;

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

		std::shared_ptr<AVFrame> AVFrameFromDecklinkVideo(IDeckLinkVideoInputFrame *decklink_frame, FieldOrder field_order, AVRational sar, BMDTimeScale time_scale);
		
		std::shared_ptr<AVFrame> AVFrameFromDecklinkAudio(IDeckLinkAudioInputPacket *audio_packet, int channels, BMDAudioSampleType sample_type, BMDTimeScale sample_rate);

		std::int64_t TimeFromDeclinkTimecode(IDeckLinkVideoInputFrame *decklink_frame, TVPlayR::DecklinkTimecodeSource timecode_source, const Common::Rational<int> &frame_rate);

		char* BMDOutputFrameCompletionResultToString(BMDOutputFrameCompletionResult result);
	}
}
