#include "../pch.h"
#include "AVDecklinkVideoFrame.h"

namespace TVPlayR {
	namespace Decklink {

AVDecklinkVideoFrame::AVDecklinkVideoFrame(FFmpeg::AVFramePtr& frame)
	:frame_(frame)
{

}

long STDMETHODCALLTYPE AVDecklinkVideoFrame::GetWidth()
{
	return frame_->width;
}

long STDMETHODCALLTYPE AVDecklinkVideoFrame::GetHeight()
{
	return frame_->height;
}

long STDMETHODCALLTYPE AVDecklinkVideoFrame::GetRowBytes()
{
	return frame_->linesize[0];
}

BMDPixelFormat STDMETHODCALLTYPE AVDecklinkVideoFrame::GetPixelFormat()
{
	switch (frame_->format)
	{
	case AV_PIX_FMT_UYVY422:
		return bmdFormat8BitYUV;
	case AV_PIX_FMT_ARGB:
		return bmdFormat8BitARGB;
	case AV_PIX_FMT_BGRA:
		return bmdFormat8BitBGRA;
	default:
		THROW_EXCEPTION("Invalid pixel format")
	}
}

BMDFrameFlags STDMETHODCALLTYPE AVDecklinkVideoFrame::GetFlags()
{
	return bmdVideoOutputFlagDefault;
}

HRESULT STDMETHODCALLTYPE AVDecklinkVideoFrame::GetBytes(void ** buffer)
{
	if (!frame_)
		return E_FAIL;
	*buffer = reinterpret_cast<void*>(frame_->data[0]);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE AVDecklinkVideoFrame::GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode ** timecode)
{
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE AVDecklinkVideoFrame::GetAncillaryData(IDeckLinkVideoFrameAncillary ** ancillary)
{
	return E_FAIL;
}


}}