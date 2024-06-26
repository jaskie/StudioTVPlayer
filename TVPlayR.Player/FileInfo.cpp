#include "stdafx.h"
#include "FileInfo.h"
#include "Rational.h"
#include "ClrStringHelper.h"
#include "FFmpeg/ThumbnailFilter.h"
#include "FFmpeg/FFmpegFileInfo.h"

namespace TVPlayR {

	FFmpeg::FFmpegFileInfo* CreateNativeFFmpegFileInfo(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
	{
		REWRAP_EXCEPTION(return new FFmpeg::FFmpegFileInfo(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice));)
	}

	FileInfo::FileInfo(String^ fileName) : FileInfo(fileName, HardwareAcceleration::None, String::Empty)
	{ }

	FileInfo::FileInfo(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
		: _nativeSource(new std::shared_ptr<FFmpeg::FFmpegFileInfo>(CreateNativeFFmpegFileInfo(fileName, acceleration, hwDevice)))
		, _fileName(fileName)
		, _acceleration(acceleration)
		, _hwDevice(hwDevice)
	{
	}

	FileInfo::~FileInfo()
	{
		this->!FileInfo();
	}

	FileInfo::!FileInfo()
	{
		if (!_nativeSource)
			return;
		delete _nativeSource;
		_nativeSource = nullptr;
	}

	TimeSpan FileInfo::AudioDuration::get() { return TimeSpan((*_nativeSource)->GetAudioDuration() * 10); }
	
	TimeSpan FileInfo::VideoDuration::get() { return TimeSpan((*_nativeSource)->GetVideoDuration() * 10); }
	
	TimeSpan FileInfo::VideoStart::get() { return TimeSpan((*_nativeSource)->GetVideoStart() * 10); }

	int FileInfo::Width::get() { return (*_nativeSource)->GetWidth(); } 

	int FileInfo::Height::get() { return (*_nativeSource)->GetHeight(); } 

	TVPlayR::FieldOrder FileInfo::FieldOrder::get() { return (*_nativeSource)->GetFieldOrder(); } 

	TVPlayR::Rational FileInfo::FrameRate::get() { return TVPlayR::Rational((*_nativeSource)->GetFrameRate()); }

	int FileInfo::AudioChannelCount::get() { return (*_nativeSource)->GetAudioChannelCount(); }
	
	bool FileInfo::HaveAlphaChannel::get() { return (*_nativeSource)->HaveAlphaChannel(); }
	
	bool FileInfo::IsStream::get() { return (*_nativeSource)->IsStream(); }


	Bitmap^ FileInfo::GetThumbnail(TimeSpan time, int width, int height)
	{
		REWRAP_EXCEPTION(
			Bitmap ^ result = nullptr;
			auto video = (*_nativeSource)->GetFrameAt(time.Ticks / 10);
			if (video == nullptr)
				return nullptr;
			FFmpeg::ThumbnailFilter filter(width, height, video);
			video = filter.Pull();
			if (video == nullptr)
				return nullptr;
			BITMAPINFOHEADER bmih;
			bmih.biSize = sizeof(BITMAPINFOHEADER);
			bmih.biWidth = video->width;
			bmih.biHeight = -video->height;
			bmih.biPlanes = 1;
			bmih.biBitCount = 24;
			bmih.biCompression = BI_RGB;
			bmih.biSizeImage = 0;
			bmih.biXPelsPerMeter = 10;
			bmih.biYPelsPerMeter = 10;
			bmih.biClrUsed = 0;
			bmih.biClrImportant = 0;

			BITMAPINFO dbmi;
			ZeroMemory(&dbmi, sizeof(dbmi));
			dbmi.bmiHeader = bmih;
			dbmi.bmiColors->rgbBlue = 0;
			dbmi.bmiColors->rgbGreen = 0;
			dbmi.bmiColors->rgbRed = 0;
			dbmi.bmiColors->rgbReserved = 0;
			void* bits = NULL;

			// Create DIB
			auto dc = ::GetDC(NULL);
			auto hBitmap = ::CreateDIBSection(dc, &dbmi, DIB_RGB_COLORS, &bits, NULL, 0);
			memcpy(bits, video->data[0], video->linesize[0] * video->height);
			result = Bitmap::FromHbitmap(IntPtr(hBitmap));
			::DeleteObject(hBitmap);
			::ReleaseDC(NULL, dc);
			return result;
			)
	}

	BitmapSource^ FileInfo::GetBitmapSource(TimeSpan time, int width, int height)
	{
		REWRAP_EXCEPTION(
		BitmapSource^ result = nullptr;
			auto video = (*_nativeSource)->GetFrameAt(time.Ticks / 10);
			if (video == nullptr)
				return nullptr;
			FFmpeg::ThumbnailFilter filter(width, height, video);
			video = filter.Pull();
			if (video == nullptr)
				return nullptr;
			return BitmapSource::Create(video->width, video->height, 96, 96, System::Windows::Media::PixelFormats::Rgb24, nullptr, IntPtr(video->data[0]), video->linesize[0] * video->height, video->linesize[0]);
			)
	}

}