#pragma once
#include "../FFmpeg/AudioFifo.h"
#include "../FFmpeg/SwResample.h"
#include "../Core/FrameTimeInfo.h"

namespace TVPlayR {
	namespace FFmpeg {
		class PlayerScaler;
	}

	namespace Core {
		class Player;
		struct AVSync;
		struct FrameTimeInfo;
		/**
		* Provides video/audio/timecode frames for player.
		* Adjust video to player framerate and resolution.
		* Resamples audio to player needs.
		*/
		class PlayerSynchroSource
		{
		public:
			const Core::Player& Player() const { return player_; }
			virtual void Push(const Core::AVSync &sync, AVRational frame_rate);
			Core::AVSync PullSync(int audio_samples_count);
			virtual ~PlayerSynchroSource();
			PlayerSynchroSource(const Core::Player &player, bool process_video, int audio_channels);
		private:
			const Core::Player &player_;
			typedef std::pair<Core::FrameTimeInfo, std::shared_ptr<AVFrame>> queue_item_t;
			std::unique_ptr<FFmpeg::PlayerScaler>		scaler_;
			const bool									process_video_;
			FFmpeg::SwResample							audio_resampler_;
			FFmpeg::AudioFifo							audio_fifo_;
			queue_item_t								last_video_;
			AVRational									input_frame_rate_;
			Common::BlockingCollection<queue_item_t>	frame_queue_;
		};
	}
}

