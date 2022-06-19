#pragma once
#include "../FFmpeg/AudioFifo.h"
#include "../FFmpeg/SwResample.h"
#include "../Core/FrameTimeInfo.h"

namespace TVPlayR {
	enum class DecklinkTimecodeSource;

	namespace Core {
		class Player;
		struct AVSync;
		struct FrameTimeInfo;
	}
	namespace FFmpeg {
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
	DecklinkInputSynchroProvider(const Core::Player& player, TVPlayR::DecklinkTimecodeSource timecode_source, bool process_video, int audio_channels);
	~DecklinkInputSynchroProvider();
	const Core::Player& Player() const;
	void Push(Core::AVSync& sync);
	Core::AVSync PullSync(int audio_samples_count);
	void Reset(AVRational input_frame_rate);
private:
	typedef std::pair<Core::FrameTimeInfo, std::shared_ptr<AVFrame>> queue_item_t;
	const Core::Player&							player_;
	std::unique_ptr<FFmpeg::PlayerScaler>		scaler_;
	const bool									process_video_;
	FFmpeg::SwResample							audio_resampler_;
	FFmpeg::AudioFifo							audio_fifo_;
	queue_item_t								last_video_;
	AVRational									input_frame_rate_;
	Common::BlockingCollection<queue_item_t>	frame_queue_;
	Common::Executor							executor_;
};

}}
