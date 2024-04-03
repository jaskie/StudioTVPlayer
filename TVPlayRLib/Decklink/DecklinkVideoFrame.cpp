#include "../pch.h"
#include "DecklinkVideoFrame.h"
#include "../Core/VideoFormat.h"
#include "../FFmpeg/FFmpegUtils.h"
#include <execution>
#include <emmintrin.h>

namespace TVPlayR {
	namespace Decklink {

		static void ConvertFrame(const std::shared_ptr<AVFrame> &source, std::vector<uint32_t> &dest)
		{
			if (source->format == AV_PIX_FMT_X2RGB10LE)
			{
				int slices[] = { 0, 1, 2, 3 };
				size_t slice_length = source->linesize[0] * source->height / (sizeof(__m128i) * (sizeof(slices) / sizeof(int)));
				std::for_each(std::execution::par, std::begin(slices), std::end(slices), [&](int slice) -> void
					{
						__m128i* src_data = reinterpret_cast<__m128i*>(source->data[0]) + (slice_length * slice);
						__m128i* dst_data = reinterpret_cast<__m128i*>(dest.data()) + (slice_length * slice);
						for (size_t i = 0; i < slice_length; i++)
						{
							__m128i data = _mm_load_si128(src_data + i);
							data = _mm_slli_epi32(data, 2);
							_mm_store_si128(dst_data + i, data);
						};
					});
			}
		}

		static BMDPixelFormat GetBMDPixelFormat(const std::shared_ptr<AVFrame> &frame)
		{
			switch (frame->format)
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

		DecklinkVideoFrame::DecklinkVideoFrame(Core::VideoFormat &format)
			: ref_count_(0)
			, width_(0)
			, height_(0)
			, row_bytes_(0)
			, pixel_format_(BMDPixelFormat(0))
			, timecode_(format)
		{ }

		DecklinkVideoFrame::~DecklinkVideoFrame()
		{
			assert(!ref_count_);
		}

		void DecklinkVideoFrame::Update(Core::VideoFormat &format, const std::shared_ptr<AVFrame> &frame, std::int64_t timecode)
		{
			timecode_.Update(format, timecode);
			width_ = format.width();
			height_ = format.height();
			row_bytes_ = frame->linesize[0];
			pixel_format_ = GetBMDPixelFormat(frame);

			if (frame->format == AV_PIX_FMT_X2RGB10LE)
			{
				buffer_.resize(width_ * height_);
				ConvertFrame(frame, buffer_);
				frame_.reset();
			}
			else
			{
				frame_ = frame;
				buffer_.clear();
			}
		}

		void DecklinkVideoFrame::Recycle()
		{
			frame_.reset();
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

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetWidth() { return width_; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetHeight() { return height_; }

		long STDMETHODCALLTYPE DecklinkVideoFrame::GetRowBytes() { return row_bytes_; }

		BMDPixelFormat STDMETHODCALLTYPE DecklinkVideoFrame::GetPixelFormat() { return pixel_format_; }

		HRESULT STDMETHODCALLTYPE DecklinkVideoFrame::GetBytes(void **buffer)
		{
			if (!buffer_.empty())
			{
				*buffer = buffer_.data();
				return S_OK;
			}
			if (frame_ && frame_->data[0])
			{
				*buffer = frame_->data[0];
				return S_OK;
			}
			return E_FAIL;
		}

		STDMETHODIMP DecklinkVideoFrame::GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode)
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