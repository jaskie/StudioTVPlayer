#include "../pch.h"
#include "DecklinkUtils.h"
#include "../FFmpeg/FFmpegUtils.h"

namespace TVPlayR {
	namespace Decklink {

		BMDPixelFormat BMDPixelFormatFromVideoFormat(const Core::PixelFormat& format)
		{
			switch (format)
			{
			case Core::PixelFormat::bgra:
				return BMDPixelFormat::bmdFormat8BitBGRA;
			case Core::PixelFormat::yuv422:
				return BMDPixelFormat::bmdFormat8BitYUV;
			default:
				break;
			}
			return (BMDPixelFormat)0;
		}

		BMDDisplayMode GetDecklinkDisplayMode(Core::VideoFormatType fmt)
		{
			switch (fmt)
			{
			case Core::VideoFormatType::pal:
			case Core::VideoFormatType::pal_fha:
				return bmdModePAL;
			case Core::VideoFormatType::ntsc:
			case Core::VideoFormatType::ntsc_fha:
				return bmdModeNTSC;
			case Core::VideoFormatType::v720p5000:		return BMDDisplayMode::bmdModeHD720p50;
			case Core::VideoFormatType::v720p5994:		return BMDDisplayMode::bmdModeHD720p5994;
			case Core::VideoFormatType::v720p6000:		return BMDDisplayMode::bmdModeHD720p60;
			case Core::VideoFormatType::v1080p2398:		return BMDDisplayMode::bmdModeHD1080p2398;
			case Core::VideoFormatType::v1080p2400:		return BMDDisplayMode::bmdModeHD1080p24;
			case Core::VideoFormatType::v1080i5000:		return BMDDisplayMode::bmdModeHD1080i50;
			case Core::VideoFormatType::v1080i5994:		return BMDDisplayMode::bmdModeHD1080i5994;
			case Core::VideoFormatType::v1080i6000:		return BMDDisplayMode::bmdModeHD1080i6000;
			case Core::VideoFormatType::v1080p2500:		return BMDDisplayMode::bmdModeHD1080p25;
			case Core::VideoFormatType::v1080p2997:		return BMDDisplayMode::bmdModeHD1080p2997;
			case Core::VideoFormatType::v1080p3000:		return BMDDisplayMode::bmdModeHD1080p30;
			case Core::VideoFormatType::v1080p5000:		return BMDDisplayMode::bmdModeHD1080p50;
			case Core::VideoFormatType::v1080p5994:		return BMDDisplayMode::bmdModeHD1080p5994;
			case Core::VideoFormatType::v1080p6000:		return BMDDisplayMode::bmdModeHD1080p6000;
			case Core::VideoFormatType::v2160p2398:		return BMDDisplayMode::bmdMode4K2160p2398;
			case Core::VideoFormatType::v2160p2400:		return BMDDisplayMode::bmdMode4K2160p24;
			case Core::VideoFormatType::v2160p2500:		return BMDDisplayMode::bmdMode4K2160p25;
			case Core::VideoFormatType::v2160p2997:		return BMDDisplayMode::bmdMode4K2160p2997;
			case Core::VideoFormatType::v2160p3000:		return BMDDisplayMode::bmdMode4K2160p30;
			case Core::VideoFormatType::v2160p5000:		return BMDDisplayMode::bmdMode4K2160p50;
			case Core::VideoFormatType::v2160p5994:		return BMDDisplayMode::bmdMode4K2160p5994;
			case Core::VideoFormatType::v2160p6000:		return BMDDisplayMode::bmdMode4K2160p60;
			default:
				return BMDDisplayMode::bmdModeUnknown;
			}
		}

		Core::VideoFormatType BMDDisplayModeToVideoFormatType(BMDDisplayMode displayMode, bool isWide)
		{
			switch (displayMode)
			{
			case BMDDisplayMode::bmdModeNTSC:			return isWide ? Core::VideoFormatType::ntsc_fha : Core::VideoFormatType::ntsc;
			case BMDDisplayMode::bmdModePAL:			return isWide ? Core::VideoFormatType::pal_fha : Core::VideoFormatType::pal;
			case BMDDisplayMode::bmdModeHD720p50:		return Core::VideoFormatType::v720p5000;
			case BMDDisplayMode::bmdModeHD720p5994:		return Core::VideoFormatType::v720p5994;
			case BMDDisplayMode::bmdModeHD720p60:		return Core::VideoFormatType::v720p6000;
			case BMDDisplayMode::bmdModeHD1080p2398:	return Core::VideoFormatType::v1080p2398;
			case BMDDisplayMode::bmdModeHD1080p24:		return Core::VideoFormatType::v1080p2400;
			case BMDDisplayMode::bmdModeHD1080p25:		return Core::VideoFormatType::v1080p2500;
			case BMDDisplayMode::bmdModeHD1080p2997:	return Core::VideoFormatType::v1080p2997;
			case BMDDisplayMode::bmdModeHD1080p30:		return Core::VideoFormatType::v1080p3000;
			case BMDDisplayMode::bmdModeHD1080i50:		return Core::VideoFormatType::v1080i5000;
			case BMDDisplayMode::bmdModeHD1080i5994:	return Core::VideoFormatType::v1080i5994;
			case BMDDisplayMode::bmdModeHD1080i6000:	return Core::VideoFormatType::v1080i6000;
			case BMDDisplayMode::bmdModeHD1080p50:		return Core::VideoFormatType::v1080p5000;
			case BMDDisplayMode::bmdModeHD1080p5994:	return Core::VideoFormatType::v1080p5994;
			case BMDDisplayMode::bmdModeHD1080p6000:	return Core::VideoFormatType::v1080p6000;
			case BMDDisplayMode::bmdMode4K2160p2398:	return Core::VideoFormatType::v2160p2398;
			case BMDDisplayMode::bmdMode4K2160p24:		return Core::VideoFormatType::v2160p2400;
			case BMDDisplayMode::bmdMode4K2160p25:		return Core::VideoFormatType::v2160p2500;
			case BMDDisplayMode::bmdMode4K2160p2997:	return Core::VideoFormatType::v2160p2997;
			case BMDDisplayMode::bmdMode4K2160p30:		return Core::VideoFormatType::v2160p3000;
			case BMDDisplayMode::bmdMode4K2160p50:		return Core::VideoFormatType::v2160p5000;
			case BMDDisplayMode::bmdMode4K2160p5994:	return Core::VideoFormatType::v2160p5994;
			case BMDDisplayMode::bmdMode4K2160p60:		return Core::VideoFormatType::v2160p6000;
			default:
				return Core::VideoFormatType::invalid;
			}
		}

		std::shared_ptr<AVFrame> AVFrameFromDecklink(IDeckLinkVideoInputFrame* decklink_frame, BMDFieldDominance field_dominance, const Common::Rational<int>& sar)
		{
			void* video_bytes = nullptr;
			if (!decklink_frame || FAILED(decklink_frame->GetBytes(&video_bytes)) && video_bytes)
				return nullptr;
			std::shared_ptr<AVFrame> frame = FFmpeg::AllocFrame();
			frame->data[0] = reinterpret_cast<uint8_t*>(video_bytes);
			frame->linesize[0] = decklink_frame->GetRowBytes();
			frame->format = AV_PIX_FMT_UYVY422;
			frame->width = decklink_frame->GetWidth();
			frame->height = decklink_frame->GetHeight();
			frame->pict_type = AV_PICTURE_TYPE_I;
			frame->interlaced_frame = field_dominance == bmdLowerFieldFirst || field_dominance == bmdUpperFieldFirst;
			frame->top_field_first = field_dominance == bmdUpperFieldFirst;
			frame->sample_aspect_ratio = sar.av();
			return frame;
		}

	}
}