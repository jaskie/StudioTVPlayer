#pragma once
#include "../FFMpeg/Utils.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace Decklink {

		class DecklinkVideoFrame : public IDeckLinkVideoFrame
		{
		private:
			const std::shared_ptr<AVFrame> frame_;
			const int64_t time_;
			ULONG ref_count_;
			Core::VideoFormat& format_;
		public:
			DecklinkVideoFrame(Core::VideoFormat& format, std::shared_ptr<AVFrame> frame, int64_t time);

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
			STDMETHOD_(int64_t, GetPts()) { return frame_->pts; }
		};

	}
}