#include "../pch.h"
#include "DecklinkInputSynchroProvider.h"
#include "DecklinkUtils.h"
#include "../Core/Channel.h"
#include "../FFmpeg/SwScale.h"
#include "../FFmpeg/AVSync.h"
#include "../FFmpeg/FFmpegUtils.h"


namespace TVPlayR {
	namespace Decklink {
		DecklinkInputSynchroProvider::DecklinkInputSynchroProvider(const Core::Channel& channel, DecklinkTimecodeSource timecode_source, bool process_video)
			: channel_(channel)
			, audio_fifo_(channel.AudioSampleFormat(), channel.AudioChannelsCount(), channel.AudioSampleRate(), av_make_q(1, channel.AudioSampleRate()), 0LL, AV_TIME_BASE/10)
			, process_video_(process_video)
		{ }

		const Core::Channel& DecklinkInputSynchroProvider::Channel() const { return channel_; }

		void DecklinkInputSynchroProvider::Push(std::shared_ptr<AVFrame> video, std::shared_ptr<AVFrame> audio, int64_t timecode)
		{
			if (video)
			{
				if (process_video_)
				{
					if (!scaler_)
						scaler_ = std::make_unique<FFmpeg::SwScale>(video->width, video->height, AV_PIX_FMT_UYVY422, channel_.Format().width(), channel_.Format().height(), Core::PixelFormatToFFmpegFormat(channel_.PixelFormat()));
					video = scaler_->Scale(video);
				}
				else
					video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
				last_video_ = video;
			}
			if (audio)
				audio_fifo_.TryPush(audio);
			last_timecode_ = timecode;
		}

		FFmpeg::AVSync DecklinkInputSynchroProvider::PullSync(int audio_samples_count)
		{
			auto video = last_video_;
			if (!video)
				video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
			auto audio = audio_fifo_.Pull(audio_samples_count);
			return FFmpeg::AVSync(audio, video, last_timecode_);
		}


}}
