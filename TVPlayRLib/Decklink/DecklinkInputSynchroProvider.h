#pragma once
#include "../FFmpeg/ChannelScaler.h"
#include "../FFmpeg/AudioFifo.h"
#include "DecklinkTimecodeSource.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {
		class AVSync;
		class ChannelScaler;
		class AudioFifo;
	}
	namespace Common {
		template<typename> class Rational;
	}
	namespace Decklink {

class DecklinkInputSynchroProvider
{
public:
	DecklinkInputSynchroProvider(const Core::Channel& channel, DecklinkTimecodeSource timecode_source);
	const Core::Channel& Channel() const;
	void Push(IDeckLinkVideoInputFrame* video_frame, IDeckLinkAudioInputPacket* audio_packet);
	FFmpeg::AVSync PullSync(int audio_samples_count);
	void SetInputParameters(BMDFieldDominance field_dominance, BMDTimeScale time_scale, BMDTimeValue frame_duration);
private:
	const Core::Channel&					channel_;
	FFmpeg::ChannelScaler					scaler_;
	FFmpeg::AudioFifo						audio_fifo_;
	std::shared_ptr<AVFrame>				last_video_;
	int64_t									frame_pts_ = 0LL;
	BMDFieldDominance						field_dominance_ = BMDFieldDominance::bmdUnknownFieldDominance;
	BMDTimeScale							time_scale_ = 1LL;
	BMDTimeValue							frame_duration_ = 0LL;
	Common::Rational<int>					frame_rate_;
	Common::Rational<int>					video_time_base_;
	DecklinkTimecodeSource					timecode_source_;
};

}}
