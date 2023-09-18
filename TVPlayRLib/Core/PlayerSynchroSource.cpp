#include "../pch.h"
#include "PlayerSynchroSource.h"
#include "../Core/AudioParameters.h"
#include "../Core/Player.h"
#include "../Core/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/PlayerScaler.h"


namespace TVPlayR {
	namespace Core {
		PlayerSynchroSource::PlayerSynchroSource(const Core::Player& player, bool process_video, int audio_channels)
			: player_(player)
			, audio_fifo_(Core::AudioParameters{ player.AudioSampleRate(), player.AudioChannelsCount(), player.AudioSampleFormat() }, av_make_q(1, player.AudioSampleRate()), 0LL, AV_TIME_BASE / 10)
			, process_video_(process_video)
			, input_frame_rate_(player.Format().FrameRate().av())
			, frame_queue_(2)
			, last_video_(Core::FrameTimeInfo(), FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat()))
			, audio_resampler_(audio_channels, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32, player.AudioChannelsCount(), player.AudioSampleRate(), player.AudioSampleFormat())
		{
		}

		void PlayerSynchroSource::Push(Core::AVSync& sync)
		{
			if (process_video_ && sync.Video)
			{
				if (!scaler_)
					scaler_ = std::make_unique<FFmpeg::PlayerScaler>(player_);
				scaler_->Push(sync.Video, input_frame_rate_, av_inv_q(input_frame_rate_));
				while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
					frame_queue_.try_add(queue_item_t(sync.TimeInfo, received_video));
			}
			if (sync.Audio)
			{
				audio_fifo_.TryPush(audio_resampler_.Resample(sync.Audio));
			}
		}

		Core::AVSync PlayerSynchroSource::PullSync(int audio_samples_count)
		{
			std::shared_ptr<AVFrame> audio;
			if (frame_queue_.try_take(last_video_) == Common::BlockingCollectionStatus::Ok)
				audio = audio_fifo_.Pull(audio_samples_count);
			else
				audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, player_.AudioChannelsCount(), player_.AudioSampleFormat());
			return Core::AVSync(audio, last_video_.second, last_video_.first);
		}

		void PlayerSynchroSource::Reset(AVRational input_frame_rate)
		{
			input_frame_rate_ = input_frame_rate;
			if (scaler_)
				scaler_->Reset();
		}

		PlayerSynchroSource::~PlayerSynchroSource() { }

	}
}
