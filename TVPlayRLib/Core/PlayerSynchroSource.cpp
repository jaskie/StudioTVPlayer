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
			: player_(player)
			, audio_fifo_(av_make_q(1, player.AudioSampleRate()), player.AudioSampleFormat(), player.AudioChannelsCount(), player.AudioSampleRate(), 0LL, AV_TIME_BASE / 10)
			, process_video_(process_video)
			, input_frame_rate_({0, 1})
			, frame_queue_(4)
			, last_video_(Core::FrameTimeInfo(), FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat()))
			, audio_resampler_(audio_channels, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32, player.AudioChannelsCount(), player.AudioSampleRate(), player.AudioSampleFormat())
		{
		}

		void PlayerSynchroSource::Push(const Core::AVSync &sync, AVRational frame_rate)
		{
			if (process_video_ && sync.Video)
			{
				if (!scaler_)
					scaler_ = std::make_unique<FFmpeg::PlayerScaler>(player_);
				if (frame_rate.num * input_frame_rate_.den != input_frame_rate_.num * frame_rate.den)
				{
					scaler_->Flush();
					while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
						frame_queue_.try_add(queue_item_t(sync.TimeInfo, received_video));
					scaler_->Clear();
					input_frame_rate_ = frame_rate;
				}
				scaler_->Push(sync.Video, frame_rate);
				while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
					frame_queue_.try_add(queue_item_t(sync.TimeInfo, received_video));
			}
			if (sync.Audio)
			{
				audio_fifo_.Push(audio_resampler_.Resample(sync.Audio));
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

		PlayerSynchroSource::~PlayerSynchroSource() { }

	}
}
