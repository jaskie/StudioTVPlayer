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
			, timecode_source_(timecode_source)
			, process_video_(process_video)
		{ }

		const Core::Channel& DecklinkInputSynchroProvider::Channel() const { return channel_; }

		void DecklinkInputSynchroProvider::Push(std::shared_ptr<AVFrame> video, IDeckLinkAudioInputPacket* audio_packet)
		{
			int64_t pts = AV_NOPTS_VALUE;
			if (video)
			{
				if (process_video_)
				{
					if (!scaler_)
						scaler_ = std::make_unique<FFmpeg::SwScale>(video->width, video->height, AV_PIX_FMT_UYVY422, channel_.Format().width(), channel_.Format().height(), Core::PixelFormatToFFmpegFormat(channel_.PixelFormat()));
					video = scaler_->Scale(video);
				}
				else
				{
					video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
					video->pts = pts;
				}
				last_video_ = video;
			}

			void* audio_bytes = nullptr;
			if (audio_packet && SUCCEEDED(audio_packet->GetBytes(&audio_bytes)) && audio_bytes)
			{
				std::shared_ptr<AVFrame> audio = FFmpeg::AllocFrame();
				audio->data[0] = reinterpret_cast<uint8_t*>(audio_bytes);
				long nb_samples = audio_packet->GetSampleFrameCount();
				audio->format = channel_.AudioSampleFormat();
				audio->nb_samples = nb_samples;
				audio->linesize[0] = nb_samples * 4;
				if (timecode_source_ == DecklinkTimecodeSource::None)
				{
					BMDTimeValue packetTime;
					if (SUCCEEDED(audio_packet->GetPacketTime(&packetTime, channel_.AudioSampleRate())))
						audio->pts = packetTime;
				}
				else
					audio->pts = pts;
				audio->channels = channel_.AudioChannelsCount();
				audio_fifo_.TryPush(audio);
			}
		}

		FFmpeg::AVSync DecklinkInputSynchroProvider::PullSync(int audio_samples_count)
		{
			auto video = last_video_;
			if (!video)
				video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
			auto audio = audio_fifo_.Pull(audio_samples_count);
			return FFmpeg::AVSync(audio, video, FFmpeg::PtsToTime(video->pts, video_time_base_.av()));
		}

		void DecklinkInputSynchroProvider::SetInputParameters(BMDFieldDominance field_dominance, BMDTimeScale time_scale, BMDTimeValue frame_duration)
		{
			field_dominance_ = field_dominance;
			time_scale_ = time_scale;
			frame_duration_ = frame_duration;
			frame_rate_ = Common::Rational<int>(static_cast<int>(time_scale), static_cast<int>(frame_duration));
			video_time_base_ = frame_rate_.invert();
		}

}}
