#include "stdafx.h"
#include "FileInput.h"
#include "ClrStringHelper.h"
#include "FFmpeg/ThumbnailFilter.h"
#include "FFmpeg/FFmpegInput.h"

namespace TVPlayR {

	FileInput::FileInput(String^ fileName) : FileInput(fileName, HardwareAcceleration::None, String::Empty)
	{ }

	FileInput::FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
		: _nativeSource(new std::shared_ptr<FFmpeg::FFmpegInput>(new FFmpeg::FFmpegInput(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice))))
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

	void FileInput::FramePlayedCallback(int64_t time)
	{
		FramePlayed(this, gcnew TimeEventArgs(TimeSpan(time * 10)));
	}

	void FileInput::StoppedCallback()
	{
		Stopped(this, EventArgs::Empty);
	}
}