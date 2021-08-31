#include "../pch.h"
#include "DecklinkSynchroProvider.h"
#include "../Core/Channel.h"
#include "../FFMpeg/AVSync.h"
#include "../FFMpeg/FFMpegUtils.h"

namespace TVPlayR {
	namespace Decklink {
		DecklinkSynchroProvider::DecklinkSynchroProvider(const Core::Channel& channel)
			: channel_(channel)
			, scaler_(channel)
		{ }

		const Core::Channel& DecklinkSynchroProvider::Channel() const { return channel_; }

		void DecklinkSynchroProvider::Push(IDeckLinkVideoInputFrame* video_frame, IDeckLinkAudioInputPacket* audio_packet)
		{
			void* bytes = nullptr;
			if (FAILED(video_frame->GetBytes(&bytes)) || !bytes)
				return;
			std::shared_ptr<AVFrame> video = FFmpeg::AllocFrame();
			video->data[0] = reinterpret_cast<uint8_t*>(bytes);
			video->linesize[0] = video_frame->GetRowBytes();
			video->format = AV_PIX_FMT_UYVY422;
			video->width = video_frame->GetWidth();
			video->height = video_frame->GetHeight();
			video->pict_type = AV_PICTURE_TYPE_I;
			video->interlaced_frame = field_dominance_ == bmdLowerFieldFirst || field_dominance_ == bmdUpperFieldFirst;
			video->top_field_first = field_dominance_ == bmdUpperFieldFirst;
			BMDTimeValue frameTime;
			BMDTimeValue frameDuration;
			if (SUCCEEDED(video_frame->GetStreamTime(&frameTime, &frameDuration, time_scale_)))
				video->pts = frameTime;
			scaler_.Push(video, frame_rate_.invert(), frame_rate_);
		}

		FFmpeg::AVSync DecklinkSynchroProvider::PullSync(int audio_samples_count)
		{
			auto video = scaler_.Pull();
			if (!video)
				video = FFmpeg::CreateEmptyVideoFrame(channel_.Format(), channel_.PixelFormat());
			auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, channel_.AudioChannelsCount(), channel_.AudioSampleFormat());
			return FFmpeg::AVSync(audio, video, 0LL);
		}

		void DecklinkSynchroProvider::SetInputParameters(BMDFieldDominance field_dominance, BMDTimeScale time_scale, BMDTimeValue frame_duration)
		{
			field_dominance_ = field_dominance;
			time_scale_ = time_scale;
			frame_duration_ = frame_duration;
			frame_rate_ = Common::Rational<int>(time_scale, frame_duration);
		}

}}
