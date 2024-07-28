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
		/*
		* Base class to provide video/audio/timecode frames for player.
		* Adjusts video and audio to player needs.
		*/
		class PlayerSynchroSource : Common::NonCopyable, protected Common::DebugTarget
		{
		public:
			const Core::Player& Player() const { return player_; }
			virtual void Push(const Core::AVSync &sync, AVRational frame_rate);
			virtual Core::AVSync PullSync(int audio_samples_count);
			void Release();
			virtual ~PlayerSynchroSource();
		protected:
			PlayerSynchroSource(const Core::Player &player, bool process_video, int audio_channels);
		private:
			const Core::Player &player_;
			typedef std::pair<Core::FrameTimeInfo, std::shared_ptr<AVFrame>> queue_item_t;
			std::unique_ptr<FFmpeg::PlayerScaler>		scaler_;
			const bool									process_video_;
			FFmpeg::SwResample							audio_resampler_;
			FFmpeg::AudioFifo							audio_fifo_;
			queue_item_t								last_video_;
			Common::BlockingCollection<queue_item_t>	video_queue_;
		};
	}
}

