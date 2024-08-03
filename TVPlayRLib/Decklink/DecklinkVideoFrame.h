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
			std::int64_t frame_number_;
			std::shared_ptr<AVFrame> frame_;
			std::vector<uint32_t> buffer_;
			int width_, height_;
			int row_bytes_;
			BMDPixelFormat pixel_format_;
			DecklinkTimecode timecode_;
			const Core::VideoFormat &format_;
		public:
			DecklinkVideoFrame(Core::VideoFormat &format);

			// Utility functions
			void Update(const std::shared_ptr<AVFrame> &frame, std::int64_t timecode, std::int64_t frame_number);
			void Recycle();
			int64_t GetFrameNumber() const { return frame_number_; }

			// IUnknown
			STDMETHOD(QueryInterface(REFIID, LPVOID*));
			STDMETHOD_(ULONG, AddRef());
			STDMETHOD_(ULONG, Release());
			
			// IDecklinkVideoFrame
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