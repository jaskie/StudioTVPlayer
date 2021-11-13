#include "../pch.h"
#include "DecklinkVideoFrame.h"
#include "../FFmpeg/FFmpegUtils.h"

namespace TVPlayR {
	namespace Decklink {
				
		DecklinkVideoFrame::DecklinkVideoFrame(Core::VideoFormat& format, std::shared_ptr<AVFrame> frame, std::int64_t timecode)
			: frame_(frame)
			, timecode_(format, timecode)
			, ref_count_(0)
			, format_(format)
		{ }

		DecklinkVideoFrame::~DecklinkVideoFrame()
		{
			assert(!ref_count_);
		}

		HRESULT STDMETHODCALLTYPE DecklinkVideoFrame::QueryInterface(REFIID, LPVOID*) { return E_NOINTERFACE; }

		ULONG STDMETHODCALLTYPE DecklinkVideoFrame::AddRef() { return InterlockedIncrement(&ref_count_); }

		ULONG STDMETHODCALLTYPE DecklinkVideoFrame::Release()
		{
			ULONG count = InterlockedDecrement(&ref_count_);
			if (count == 0)
				delete this;
			return count;
		}

		BMDFrameFlags STDMETHODCALLTYPE DecklinkVideoFrame::GetFlags() { return bmdVideoOutputFlagDefault; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetWidth() { return frame_->width; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetHeight() { return frame_->height; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetRowBytes() { return frame_->linesize[0]; }

		BMDPixelFormat STDMETHODCALLTYPE DecklinkVideoFrame::GetPixelFormat()
		{
			switch (frame_->format)
			{
			case AV_PIX_FMT_ARGB:
				return BMDPixelFormat::bmdFormat8BitARGB;
			case AV_PIX_FMT_BGRA:
				return BMDPixelFormat::bmdFormat8BitBGRA;
			case AV_PIX_FMT_UYVY422:
				return BMDPixelFormat::bmdFormat8BitYUV;
			default:
				return BMDPixelFormat(0);
			}
		}

		HRESULT STDMETHODCALLTYPE DecklinkVideoFrame::GetBytes(void** buffer)
		{
			if (!frame_ || !frame_->data[0])
				return E_FAIL;
			*buffer = frame_->data[0];
			return S_OK;
		}

		STDMETHODIMP DecklinkVideoFrame::GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode** timecode)
		{
			if (timecode == nullptr)
				return E_FAIL;
			*timecode = &timecode_;
			return S_OK;
		}

	}
}