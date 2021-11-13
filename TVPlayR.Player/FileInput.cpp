#include "stdafx.h"
#include "FileInput.h"
#include "ClrStringHelper.h"
#include "FFmpeg/ThumbnailFilter.h"
#include "FFmpeg/FFmpegInput.h"

namespace TVPlayR {

	FileInput::FileInput(String^ fileName) : FileInput(fileName, HardwareAcceleration::None, String::Empty)
	{ }

	FileInput::FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
		: InputBase(std::shared_ptr<Core::InputSource>(new FFmpeg::FFmpegInput(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice))))
		, _fileName(fileName)
		, _acceleration(acceleration)
		, _hwDevice(hwDevice)
	{ 
	
		_stoppedDelegate = gcnew StoppedDelegate(this, &FileInput::StoppedCallback);
		_stoppedHandle = GCHandle::Alloc(_stoppedDelegate);
		IntPtr stoppedIp = Marshal::GetFunctionPointerForDelegate(_stoppedDelegate);
		typedef void(__stdcall* STOPPED_CALLBACK) (); // compatible with Core::InputSource::STOPPED_CALLBACK
		GetFFmpegInput()->SetStoppedCallback(static_cast<STOPPED_CALLBACK>(stoppedIp.ToPointer()));
	}

	FileInput::~FileInput()
	{
		this->!FileInput();
	}

	FileInput::!FileInput()
	{
		std::shared_ptr<FFmpeg::FFmpegInput> input = GetFFmpegInput();
		if (!input)
			return;
		input->SetStoppedCallback(nullptr);
		_stoppedHandle.Free();
	}

	bool FileInput::Seek(TimeSpan time)
	{
		return GetFFmpegInput()->Seek(time.Ticks / 10);
	}

	void FileInput::Play()
	{
		GetFFmpegInput()->Play();
	}

	void FileInput::Pause()
	{
		GetFFmpegInput()->Pause();
	}

	void FileInput::StoppedCallback()
	{
		Stopped(this, EventArgs::Empty);
	}
}