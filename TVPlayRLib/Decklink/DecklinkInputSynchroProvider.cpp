#include "../pch.h"
#include "DecklinkInputSynchroProvider.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkUtils.h"
#include "../Core/Player.h"
#include "../FFmpeg/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/PlayerScaler.h"

namespace TVPlayR {
	namespace Decklink {
		DecklinkInputSynchroProvider::DecklinkInputSynchroProvider(const Core::Player& player, TVPlayR::DecklinkTimecodeSource timecode_source, bool process_video)
			: player_(player)
			, audio_fifo_(player.AudioSampleFormat(), player.AudioChannelsCount(), player.AudioSampleRate(), av_make_q(1, player.AudioSampleRate()), 0LL, AV_TIME_BASE/10)
			, process_video_(process_video)
			, input_frame_rate_(player.Format().FrameRate().av())
			, frame_queue_(2)
			, last_video_(AV_NOPTS_VALUE, FFmpeg::CreateEmptyVideoFrame(player.Format(), player.PixelFormat()))
			, executor_("Input provider for " + player.Name())
		{
		}

		DecklinkInputSynchroProvider::~DecklinkInputSynchroProvider()
		{
		}

		const Core::Player& DecklinkInputSynchroProvider::Player() const { return player_; }

		void DecklinkInputSynchroProvider::Push(const std::shared_ptr<AVFrame>& video, const std::shared_ptr<AVFrame>& audio, std::int64_t timecode)
		{
			executor_.begin_invoke([=] {
				if (process_video_ && video)
				{
					if (!scaler_)
						scaler_ = std::make_unique<FFmpeg::PlayerScaler>(player_);
					scaler_->Push(video, input_frame_rate_, av_inv_q(input_frame_rate_));
					while (std::shared_ptr<AVFrame> received_video = scaler_->Pull())
						frame_queue_.try_add(queue_item_t(timecode, received_video));
				}
				if (audio)
					audio_fifo_.TryPush(audio);
			});
		}

		FFmpeg::AVSync DecklinkInputSynchroProvider::PullSync(int audio_samples_count)
		{
			frame_queue_.try_take(last_video_);
			auto audio = audio_fifo_.Pull(audio_samples_count);
			return FFmpeg::AVSync(audio, last_video_.second, last_video_.first);
		}

		void DecklinkInputSynchroProvider::Reset(AVRational input_frame_rate)
		{
			input_frame_rate_ = input_frame_rate;
			if (scaler_)
				scaler_->Reset();
		}



}}
