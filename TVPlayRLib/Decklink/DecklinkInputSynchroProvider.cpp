#include "../pch.h"
#include "DecklinkInputSynchroProvider.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkUtils.h"
#include "../Core/Player.h"
#include "../Core/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/PlayerScaler.h"

namespace TVPlayR {
	namespace Decklink {
		DecklinkInputSynchroProvider::DecklinkInputSynchroProvider(const Core::Player& player, TVPlayR::DecklinkTimecodeSource timecode_source, bool process_video, int audio_channels)
			: player_(player)
			, audio_fifo_(player.AudioSampleFormat(), player.AudioChannelsCount(), player.AudioSampleRate(), av_make_q(1, player.AudioSampleRate()), 0LL, AV_TIME_BASE/10)
			, process_video_(process_video)
			, input_frame_rate_(player.Format().FrameRate().av())
			, frame_queue_(2)
			, last_video_(AV_NOPTS_VALUE, FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat()))
			, audio_resampler_(audio_channels, bmdAudioSampleRate48kHz, AVSampleFormat::AV_SAMPLE_FMT_S32, player.AudioChannelsCount(), player.AudioSampleRate(), player.AudioSampleFormat())
			, executor_("Input provider for " + player.Name())
		{
		}

		DecklinkInputSynchroProvider::~DecklinkInputSynchroProvider()
		{
		}

		const Core::Player& DecklinkInputSynchroProvider::Player() const { return player_; }

		void DecklinkInputSynchroProvider::Push(Core::AVSync& sync)
		{
			executor_.begin_invoke([=] {
				if (process_video_ && sync.Video)
				{
					if (!scaler_)
						scaler_ = std::make_unique<FFmpeg::PlayerScaler>(player_);
					scaler_->Push(sync.Video, input_frame_rate_, av_inv_q(input_frame_rate_));
					while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
						frame_queue_.try_add(queue_item_t(sync.TimeInfo.Timecode, received_video));
				}
				if (sync.Audio)
				{
					audio_fifo_.TryPush(audio_resampler_.Resample(sync.Audio));
				}
			});
		}

		Core::AVSync DecklinkInputSynchroProvider::PullSync(int audio_samples_count)
		{
			frame_queue_.try_take(last_video_);
			auto audio = audio_fifo_.Pull(audio_samples_count);
			return Core::AVSync(audio, last_video_.second, Core::FrameTimeInfo { last_video_.first, AV_NOPTS_VALUE, AV_NOPTS_VALUE });
		}

		void DecklinkInputSynchroProvider::Reset(AVRational input_frame_rate)
		{
			input_frame_rate_ = input_frame_rate;
			if (scaler_)
				scaler_->Reset();
		}



}}
