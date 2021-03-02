#pragma once
#include "../FFMpeg/Utils.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace Decklink {

		class DecklinkVideoFrame : public IDeckLinkVideoFrame
		{
		private:
			const std::shared_ptr<AVFrame> frame_;
			ULONG ref_count_;
		public:
			DecklinkVideoFrame(std::shared_ptr<AVFrame> frame);

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
			STDMETHOD(GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode** timecode)) { return E_FAIL; }
			STDMETHOD(GetAncillaryData(IDeckLinkVideoFrameAncillary** ancillary)) { return E_FAIL; }

			//inne
			STDMETHOD_(int64_t, GetPts()) { return frame_->pts; }
		};

	}
}