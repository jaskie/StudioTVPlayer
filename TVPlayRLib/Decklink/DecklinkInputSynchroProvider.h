#pragma once
#include "../Common/Rational.h"
#include "../Common/BlockingCollection.h"
#include "../Common/Executor.h"
#include "../FFmpeg/AudioFifo.h"
#include "../FFmpeg/ChannelScaler.h"
#include "../DecklinkTimecodeSource.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {
		class AVSync;
	}
	namespace Common {
		template<typename> class Rational;
	}
	namespace Decklink {

class DecklinkInputSynchroProvider
{
public:
	DecklinkInputSynchroProvider(const Core::Channel& channel, TVPlayR::DecklinkTimecodeSource timecode_source, bool process_video);
	const Core::Channel& Channel() const;
	void Push(const std::shared_ptr<AVFrame>& video, const std::shared_ptr<AVFrame>& audio, int64_t timecode);
	FFmpeg::AVSync PullSync(int audio_samples_count);
	void Reset(AVRational input_frame_rate);
private:
	typedef std::pair<int64_t, std::shared_ptr<AVFrame>> queue_item_t;
	const Core::Channel&						channel_;
	std::unique_ptr<FFmpeg::ChannelScaler>		scaler_;
	const bool									process_video_;
	FFmpeg::AudioFifo							audio_fifo_;
	queue_item_t								last_video_;
	AVRational									input_frame_rate_;
	Common::BlockingCollection<queue_item_t>	frame_queue_;
	Common::Executor							executor_;
};

}}
