#include "../pch.h"
#include "DecklinkVideoFrame.h"
#include "../Core/VideoFormat.h"
#include "../FFmpeg/FFmpegUtils.h"
#include <execution>
#include <emmintrin.h>

namespace TVPlayR {
	namespace Decklink {
		
		std::shared_ptr<AVFrame> ConvertFrame(const std::shared_ptr<AVFrame>& source)
		{
			if (source->format == AV_PIX_FMT_X2RGB10LE)
			{
				int slices[] = { 0, 1, 2, 3 };
				size_t slice_length = source->linesize[0] * source->height / (sizeof(__m128i) * (sizeof(slices) / sizeof(int)));
				std::for_each(std::execution::par, std::begin(slices), std::end(slices), [&](int slice) -> void
					{
						__m128i* src_data = reinterpret_cast<__m128i*>(source->data[0]) + (slice_length * slice);
						for (size_t i = 0; i < slice_length; i++)
						{
							__m128i data = _mm_load_si128(src_data + i);
							data = _mm_slli_epi32(data, 2);
							_mm_store_si128(src_data + i, data);
						};
					});
			}
			return source;
		}

		DecklinkVideoFrame::DecklinkVideoFrame(Core::VideoFormat& format, const std::shared_ptr<AVFrame>& frame, std::int64_t timecode)
			: frame_(ConvertFrame(frame))
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

		BMDFrameFlags STDMETHODCALLTYPE DecklinkVideoFrame::GetFlags() { return timecode_.IsValid() ? BMDVideoOutputFlags::bmdVideoOutputRP188 | BMDVideoOutputFlags::bmdVideoOutputVITC : BMDVideoOutputFlags::bmdVideoOutputFlagDefault; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetWidth() { return format_.width(); }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetHeight() { return format_.height(); }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetRowBytes() { return frame_->linesize[0]; }

		BMDPixelFormat STDMETHODCALLTYPE DecklinkVideoFrame::GetPixelFormat()
		{
			switch (frame_->format)
			{
			case AV_PIX_FMT_BGRA:
				return BMDPixelFormat::bmdFormat8BitBGRA;
			case AV_PIX_FMT_UYVY422:
				return BMDPixelFormat::bmdFormat8BitYUV;
			case AV_PIX_FMT_X2RGB10LE:
				return BMDPixelFormat::bmdFormat10BitRGBXLE;
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
			if (!timecode_.IsValid())
				return E_FAIL;
			*timecode = &timecode_;
			return S_OK;
		}

	}
}