#include "../pch.h"
#include "DecklinkSynchroProvider.h"
#include "../Core/Channel.h"
#include "../FFMpeg/AVSync.h"
#include "DecklinkChannelScaler.h"


namespace TVPlayR {
	namespace Decklink {
		DecklinkSynchroProvider::DecklinkSynchroProvider(const Core::Channel* channel)
			: channel_(channel)
			//, frame_pts_(0LL)
		{
		}

		DecklinkSynchroProvider::~DecklinkSynchroProvider()
		{
		}

		const Core::Channel* DecklinkSynchroProvider::Channel() const { return channel_; }

		void DecklinkSynchroProvider::Push(IDeckLinkVideoInputFrame* videoFrame, BMDFieldDominance fieldDominance, IDeckLinkAudioInputPacket* audioPacket)
		{
			void* bytes = nullptr;
			if (FAILED(videoFrame->GetBytes(&bytes)) || !bytes)
				return;
			std::shared_ptr<AVFrame> video = FFmpeg::AllocFrame();
			video->data[0] = reinterpret_cast<uint8_t*>(bytes);
			video->linesize[0] = videoFrame->GetRowBytes();
			video->format = AV_PIX_FMT_UYVY422;
			video->width = videoFrame->GetWidth();
			video->height = videoFrame->GetHeight();
			video->pict_type = AV_PICTURE_TYPE_I;
			video->interlaced_frame = fieldDominance == bmdLowerFieldFirst || fieldDominance == bmdUpperFieldFirst;
			video->top_field_first = fieldDominance == bmdUpperFieldFirst;
			video->pts = frame_pts_++;
			//scaler_->Push(video);
		}

		FFmpeg::AVSync DecklinkSynchroProvider::PullSync(int audioSamplesCount)
		{
			return FFmpeg::AVSync();
		}

}}
