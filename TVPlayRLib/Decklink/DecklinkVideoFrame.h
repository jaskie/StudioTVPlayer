#pragma once
#include "DecklinkTimecode.h"

namespace TVPlayR {
	namespace Core	{
		class VideoFormat;
	}
	namespace Decklink {

		class DecklinkVideoFrame final : public IDeckLinkVideoFrame
		{
		private:
			const std::shared_ptr<AVFrame> frame_;
			ULONG ref_count_;
			Core::VideoFormat& format_;
			DecklinkTimecode timecode_;
		public:
			DecklinkVideoFrame(Core::VideoFormat& format, std::shared_ptr<AVFrame> frame, std::int64_t timecode);
			~DecklinkVideoFrame();
			//IUnknown
			STDMETHOD(QueryInterface(REFIID, LPVOID*));
			STDMETHOD_(ULONG, AddRef());
			STDMETHOD_(ULONG, Release());
			
			//IDecklinkVideoFrame
			STDMETHOD_(long, GetWidth());
			STDMETHOD_(long, GetHeight());
			STDMETHOD_(long, GetRowBytes());
			STDMETHOD_(BMDPixelFormat, GetPixelFormat());
			STDMETHOD_(BMDFrameFlags, GetFlags());
			STDMETHOD(GetBytes(void** buffer));
			STDMETHOD(GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode** timecode));
			STDMETHOD(GetAncillaryData(IDeckLinkVideoFrameAncillary** ancillary)) { return E_FAIL; }

			//other
			STDMETHOD_(std::int64_t, GetPts()) { return frame_->pts; }
		};

	}
}