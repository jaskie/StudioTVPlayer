#include "stdafx.h"
#include "InputFile.h"
#include "ClrStringHelper.h"
#include "FFMpeg/ThumbnailFilter.h"
#include "FFMpeg/FFMpegInputSource.h"

namespace TVPlayR {



	InputFile::InputFile(String^ fileName, int audioChannelCount) : InputFile(fileName, HardwareAcceleration::None, String::Empty, audioChannelCount)
	{ }

	InputFile::InputFile(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice, int audioChannelCount)
		: _nativeSource(new FFmpeg::FFmpegInputSource(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice), audioChannelCount))
		, _fileName(fileName)
		, _acceleration(acceleration)
		, _hwDevice(hwDevice)
	{ 
		_framePlayedDelegate = gcnew FramePlayedDelegate(this, &InputFile::FramePlayedCallback);
		_framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
		IntPtr framePlayedIp =  Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
		_nativeSource->SetFramePlayedCallback(static_cast<Core::InputSource::TIME_CALLBACK>(framePlayedIp.ToPointer()));
		
		_stoppedDelegate = gcnew StoppedDelegate(this, &InputFile::StoppedCallback);
		_stoppedHandle = GCHandle::Alloc(_stoppedDelegate);
		IntPtr stoppedIp = Marshal::GetFunctionPointerForDelegate(_stoppedDelegate);
		_nativeSource->SetStoppedCallback(static_cast<Core::InputSource::STOPPED_CALLBACK>(stoppedIp.ToPointer()));
	}

	InputFile::~InputFile()
	{
		this->!InputFile();
		_framePlayedHandle.Free();
		_stoppedHandle.Free();
	}

	InputFile::!InputFile()
	{
		delete _nativeSource;
	}

	bool InputFile::Seek(TimeSpan time)
	{
		return _nativeSource->Seek(time.Ticks / 10);
	}

	void InputFile::Play()
	{
		_nativeSource->Play();
	}

	void InputFile::Pause()
	{
		_nativeSource->Pause();
	}

	Bitmap ^ InputFile::GetThumbnail(TimeSpan time, int height)
	{
		Bitmap^ result = nullptr;
		try
		{
			auto video = _nativeSource->GetFrameAt(time.Ticks / 10);
			if (video == nullptr)
				return nullptr;
			FFmpeg::ThumbnailFilter filter(height, video);
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
		}
		catch(...)
		{
		}
		return result;
	}

	BitmapSource ^ InputFile::GetBitmapSource(TimeSpan time, int height)
	{
		BitmapSource^ result = nullptr;
		try
		{
			auto video = _nativeSource->GetFrameAt(time.Ticks / 10);
			if (video == nullptr)
				return nullptr;
			FFmpeg::ThumbnailFilter filter(height, video);
			video = filter.Pull();
			if (video == nullptr)
				return nullptr;

			result = BitmapSource::Create(video->width, video->height, 96, 96, Windows::Media::PixelFormats::Rgb24, nullptr, IntPtr(video->data[0]), video->linesize[0] * video->height, video->linesize[0]);

		}
		catch (...)
		{
		}
		return result;
	}

	void InputFile::FramePlayedCallback(int64_t time)
	{
		FramePlayed(this, gcnew TimeEventArgs(TimeSpan(time * 10)));
	}

	void InputFile::StoppedCallback()
	{
		Stopped(this, EventArgs::Empty);
	}
}