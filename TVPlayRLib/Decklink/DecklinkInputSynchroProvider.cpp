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

		void DecklinkInputSynchroProvider::Push(IDeckLinkVideoInputFrame* video_frame, IDeckLinkAudioInputPacket* audio_packet)
		{
			int64_t pts = AV_NOPTS_VALUE;
			switch (timecode_source_)
			{
			case DecklinkTimecodeSource::RP188Any:
				pts = GetPts(video_frame, BMDTimecodeFormat::bmdTimecodeRP188Any);
				break;
			case DecklinkTimecodeSource::VITC:
				pts = GetPts(video_frame, BMDTimecodeFormat::bmdTimecodeVITC);
				break;
			default:
				BMDTimeValue frameTime;
				BMDTimeValue frameDuration;
				if (SUCCEEDED(video_frame->GetStreamTime(&frameTime, &frameDuration, time_scale_)))
					pts = frameTime / frameDuration;
				break;
			}

			std::shared_ptr<AVFrame> video;
			if (process_video_)
			{
				video = AVFrameFromDecklink(video_frame, field_dominance_, channel_.Format().SampleAspectRatio());
				video->pts = pts;
				if (!scaler_)
					scaler_ = std::make_unique<FFmpeg::SwScale>(video_frame->GetWidth(), video_frame->GetHeight(), AV_PIX_FMT_UYVY422, channel_.Format().width(), channel_.Format().height(), Core::PixelFormatToFFmpegFormat(channel_.PixelFormat()));
				video = scaler_->Scale(video);
			}
			else
			{
				video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
				video->pts = pts;
			}
			last_video_ = video;

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

		int64_t DecklinkInputSynchroProvider::GetPts(IDeckLinkVideoInputFrame* video_frame, BMDTimecodeFormat timecode_format)
		{
			CComPtr<IDeckLinkTimecode> timecode;
			if (SUCCEEDED(video_frame->GetTimecode(timecode_format, &timecode)))
			{
				unsigned char hours, minutes, seconds, frames;
				if (timecode && SUCCEEDED(timecode->GetComponents(&hours, &minutes, &seconds, &frames)))
					return ((((hours * 60) + minutes) * 60) + seconds) * frame_rate_.Numerator() / frame_rate_.Denominator() + frames;
			}
			return AV_NOPTS_VALUE;
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
