#pragma once
#include "../Core/PixelFormat.h"
#include "../Core/VideoFormat.h"
#include "DeckLinkAPI_h.h"

namespace TVPlayR {
	namespace Decklink {

		static BMDPixelFormat BMDPixelFormatFromVideoFormat(const Core::PixelFormat& format)
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

		static BMDDisplayMode GetDecklinkVideoFormat(Core::VideoFormatType fmt)
		{
			switch (fmt)
			{
			case Core::VideoFormatType::pal:
			case Core::VideoFormatType::pal_fha:
				return bmdModePAL;
			case Core::VideoFormatType::ntsc:
			case Core::VideoFormatType::ntsc_fha:
				return bmdModeNTSC;
			case Core::VideoFormatType::v1080p2398:		return bmdModeHD1080p2398;
			case Core::VideoFormatType::v1080p2400:		return bmdModeHD1080p24;
			case Core::VideoFormatType::v1080i5000:		return bmdModeHD1080i50;
			case Core::VideoFormatType::v1080i5994:		return bmdModeHD1080i5994;
			case Core::VideoFormatType::v1080i6000:		return bmdModeHD1080i6000;
			case Core::VideoFormatType::v1080p2500:		return bmdModeHD1080p25;
			case Core::VideoFormatType::v1080p2997:		return bmdModeHD1080p2997;
			case Core::VideoFormatType::v1080p3000:		return bmdModeHD1080p30;
			case Core::VideoFormatType::v1080p5000:		return bmdModeHD1080p50;
			case Core::VideoFormatType::v1080p5994:		return bmdModeHD1080p5994;
			case Core::VideoFormatType::v1080p6000:		return bmdModeHD1080p6000;
			case Core::VideoFormatType::v2160p2398:		return bmdMode4K2160p2398;
			case Core::VideoFormatType::v2160p2400:		return bmdMode4K2160p24;
			case Core::VideoFormatType::v2160p2500:		return bmdMode4K2160p25;
			case Core::VideoFormatType::v2160p2997:		return bmdMode4K2160p2997;
			case Core::VideoFormatType::v2160p3000:		return bmdMode4K2160p30;
			case Core::VideoFormatType::v2160p5000:		return bmdMode4K2160p50;
			case Core::VideoFormatType::v2160p6000:		return bmdMode4K2160p60;
			default:
				return (BMDDisplayMode)ULONG_MAX;
			}
		}
	}
}
