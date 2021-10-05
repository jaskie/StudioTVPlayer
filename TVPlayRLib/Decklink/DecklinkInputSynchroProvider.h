#pragma once
#include "../Common/Rational.h"
#include "../FFmpeg/SwScale.h"
#include "../FFmpeg/AudioFifo.h"
#include "DecklinkTimecodeSource.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {
		class AVSync;
		class SwScale;
		class AudioFifo;
	}
	namespace Common {
		template<typename> class Rational;
	}
	namespace Decklink {

class DecklinkInputSynchroProvider
{
public:
	DecklinkInputSynchroProvider(const Core::Channel& channel, DecklinkTimecodeSource timecode_source, bool process_video);
	const Core::Channel& Channel() const;
	void Push(std::shared_ptr<AVFrame> video, std::shared_ptr<AVFrame> audio, int64_t timecode);
	FFmpeg::AVSync PullSync(int audio_samples_count);
private:
	const Core::Channel&					channel_;
	std::unique_ptr<FFmpeg::SwScale>		scaler_;
	const bool								process_video_;
	FFmpeg::AudioFifo						audio_fifo_;
	std::shared_ptr<AVFrame>				last_video_;
	int64_t									last_timecode_;
};

}}
