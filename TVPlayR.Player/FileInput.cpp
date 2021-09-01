#include "stdafx.h"
#include "FileInput.h"
#include "ClrStringHelper.h"
#include "FFMpeg/ThumbnailFilter.h"
#include "FFMpeg/FFMpegInputSource.h"

namespace TVPlayR {

	FileInput::FileInput(String^ fileName, int audioChannelCount) : FileInput(fileName, HardwareAcceleration::None, String::Empty, audioChannelCount)
	{ }

	FileInput::FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice, int audioChannelCount)
		: _nativeSource(new std::shared_ptr<FFmpeg::FFmpegInputSource>(new FFmpeg::FFmpegInputSource(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice), audioChannelCount)))
		, _fileName(fileName)
		, _acceleration(acceleration)
		, _hwDevice(hwDevice)
	{ 
		_framePlayedDelegate = gcnew FramePlayedDelegate(this, &FileInput::FramePlayedCallback);
		_framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
		IntPtr framePlayedIp =  Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
		(*_nativeSource)->SetFramePlayedCallback(static_cast<Core::InputSource::TIME_CALLBACK>(framePlayedIp.ToPointer()));
		
		_stoppedDelegate = gcnew StoppedDelegate(this, &FileInput::StoppedCallback);
		_stoppedHandle = GCHandle::Alloc(_stoppedDelegate);
		IntPtr stoppedIp = Marshal::GetFunctionPointerForDelegate(_stoppedDelegate);
		(*_nativeSource)->SetStoppedCallback(static_cast<Core::InputSource::STOPPED_CALLBACK>(stoppedIp.ToPointer()));
	}

	FileInput::~FileInput()
	{
		this->!FileInput();
	}

	FileInput::!FileInput()
	{
		if (!_nativeSource)
			return;
		(*_nativeSource)->SetFramePlayedCallback(nullptr);
		(*_nativeSource)->SetStoppedCallback(nullptr);
		_framePlayedHandle.Free();
		_stoppedHandle.Free();
		delete _nativeSource;
		_nativeSource = nullptr;
	}

	bool FileInput::Seek(TimeSpan time)
	{
		return (*_nativeSource)->Seek(time.Ticks / 10);
	}

	void FileInput::Play()
	{
		(*_nativeSource)->Play();
	}

	void FileInput::Pause()
	{
		(*_nativeSource)->Pause();
	}

	Bitmap ^ FileInput::GetThumbnail(TimeSpan time, int width, int height)
	{
		Bitmap^ result = nullptr;
		try
		{
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
		}
		catch(...)
		{
		}
		return result;
	}

	BitmapSource ^ FileInput::GetBitmapSource(TimeSpan time, int width, int height)
	{
		BitmapSource^ result = nullptr;
		try
		{
			auto video = (*_nativeSource)->GetFrameAt(time.Ticks / 10);
			if (video == nullptr)
				return nullptr;
			FFmpeg::ThumbnailFilter filter(width, height, video);
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

	void FileInput::FramePlayedCallback(int64_t time)
	{
		FramePlayed(this, gcnew TimeEventArgs(TimeSpan(time * 10)));
	}

	void FileInput::StoppedCallback()
	{
		Stopped(this, EventArgs::Empty);
	}
}