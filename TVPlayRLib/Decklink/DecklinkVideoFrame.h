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
			std::shared_ptr<AVFrame> frame_;
			std::vector<uint32_t> buffer_;
			ULONG ref_count_;
			int width_, height_;
			int row_bytes_;
			BMDPixelFormat pixel_format_;
			std::shared_ptr<DecklinkTimecode> timecode_;
		public:
			DecklinkVideoFrame();
			virtual ~DecklinkVideoFrame();

			void Update(Core::VideoFormat& format, const std::shared_ptr<AVFrame>& frame, std::int64_t timecode);
			void Recycle();
		
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

		};

	}
}