#pragma once
#include "../FFmpeg/AudioFifo.h"

namespace TVPlayR {
	enum class DecklinkTimecodeSource;

	namespace Core {
		class Player;
	}
	namespace FFmpeg {
		class AVSync;
		class PlayerScaler;
	}
	namespace Common {
		template<typename> class Rational;
		class Executor;
	}
	namespace Decklink {

class DecklinkInputSynchroProvider
{
public:
	DecklinkInputSynchroProvider(const Core::Player& player, TVPlayR::DecklinkTimecodeSource timecode_source, bool process_video);
	~DecklinkInputSynchroProvider();
	const Core::Player& Player() const;
	void Push(const std::shared_ptr<AVFrame>& video, const std::shared_ptr<AVFrame>& audio, std::int64_t timecode);
	FFmpeg::AVSync PullSync(int audio_samples_count);
	void Reset(AVRational input_frame_rate);
private:
	typedef std::pair<std::int64_t, std::shared_ptr<AVFrame>> queue_item_t;
	const Core::Player&						player_;
	std::unique_ptr<FFmpeg::PlayerScaler>		scaler_;
	const bool									process_video_;
	FFmpeg::AudioFifo							audio_fifo_;
	queue_item_t								last_video_;
	AVRational									input_frame_rate_;
	Common::BlockingCollection<queue_item_t>	frame_queue_;
	Common::Executor							executor_;
};

}}
