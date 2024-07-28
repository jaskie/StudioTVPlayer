#include "../pch.h"
#include "PlayerSynchroSource.h"
#include "AudioParameters.h"
#include "Player.h"
#include "AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/PlayerScaler.h"


namespace TVPlayR {
	namespace Core {
		PlayerSynchroSource::PlayerSynchroSource(const Core::Player &player, bool process_video, int audio_channels)
			: Common::DebugTarget(Common::DebugSeverity::info, "PlayerSynchroSource for " + player.Name())
			, player_(player)
			, audio_fifo_(FFmpeg::AudioFifo(av_make_q(1, player.AudioSampleRate()), player.AudioSampleFormat(), player.AudioChannelsCount(), player.AudioSampleRate(), 0LL, AV_TIME_BASE / 10, player.Name()))
			, process_video_(process_video)
			, video_queue_(2)
			, last_video_(Core::FrameTimeInfo(), FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat()))
			, audio_resampler_(audio_channels, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32, player.AudioChannelsCount(), player.AudioSampleRate(), player.AudioSampleFormat())
		{
		}

		void PlayerSynchroSource::Push(const Core::AVSync &sync, AVRational frame_rate)
		{
			if (process_video_ && sync.Video)
			{
				if (!scaler_)
				{
					scaler_ = std::make_unique<FFmpeg::PlayerScaler>(player_);
					scaler_->SetInputFrameRate(frame_rate);
				}
				if (av_cmp_q(frame_rate, scaler_->GetInputFrameRate()) != 0) // frame rate changed - read remaining frames and clear scaler
				{
					scaler_->Flush();
					while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
						video_queue_.emplace(sync.TimeInfo, received_video);
					scaler_->SetInputFrameRate(frame_rate);
				}
				scaler_->Push(sync.Video);
				while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
					video_queue_.emplace(sync.TimeInfo, received_video);
			}
			if (sync.Audio)
			{
				audio_fifo_.Push(audio_resampler_.Resample(sync.Audio));
			}
		}

		Core::AVSync PlayerSynchroSource::PullSync(int audio_samples_count)
		{
			std::shared_ptr<AVFrame> audio;
			if (video_queue_.try_take(last_video_) == Common::BlockingCollectionStatus::Ok)
				audio = audio_fifo_.Pull(audio_samples_count);
			else
				audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, player_.AudioChannelsCount(), player_.AudioSampleFormat());
			return Core::AVSync(audio, last_video_.second, last_video_.first);
		}

		void PlayerSynchroSource::Release()
		{
			video_queue_.complete_adding();
		}

		PlayerSynchroSource::~PlayerSynchroSource() { }

	}
}
