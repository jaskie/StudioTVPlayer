#pragma once
#include "../FFMpeg/Utils.h"

namespace TVPlayR {
	namespace Decklink {

class AVDecklinkVideoFrame: public IDeckLinkVideoFrame
{
private:
	const FFmpeg::AVFramePtr frame_;
public:
	AVDecklinkVideoFrame(FFmpeg::AVFramePtr& frame);

	//IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
	ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
	ULONG STDMETHODCALLTYPE Release() override { return 1; }

	//IDecklinkVideoFrame
	virtual long STDMETHODCALLTYPE GetWidth();
	virtual long STDMETHODCALLTYPE GetHeight();
	virtual long STDMETHODCALLTYPE GetRowBytes();
	virtual BMDPixelFormat STDMETHODCALLTYPE GetPixelFormat();
	virtual BMDFrameFlags STDMETHODCALLTYPE GetFlags();
	virtual HRESULT STDMETHODCALLTYPE GetBytes(void **buffer);
	virtual HRESULT STDMETHODCALLTYPE GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode);
	virtual HRESULT STDMETHODCALLTYPE GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary);
};

}}