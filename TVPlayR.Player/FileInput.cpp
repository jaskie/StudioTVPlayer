#include "stdafx.h"
#include "FileInput.h"
#include "Rational.h"
#include "ClrStringHelper.h"
#include "FFmpeg/ThumbnailFilter.h"
#include "FFmpeg/FFmpegInput.h"
#include "FieldOrder.h"

namespace TVPlayR {

	FFmpeg::FFmpegInput* CreateNativeFFmpegInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
	{
		REWRAP_EXCEPTION(return new FFmpeg::FFmpegInput(ClrStringToStdString(fileName), static_cast<Core::HwAccel>(acceleration), ClrStringToStdString(hwDevice));)
	}

	FileInput::FileInput(String^ fileName) : FileInput(fileName, HardwareAcceleration::None, String::Empty)
	{ }

	FileInput::FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice)
		: InputBase(std::shared_ptr<Core::InputSource>(CreateNativeFFmpegInput(fileName, acceleration, hwDevice)))
		, _fileName(fileName)
		, _acceleration(acceleration)
		, _hwDevice(hwDevice)
	{ 
		_pausedDelegate = gcnew PausedDelegate(this, &FileInput::PausedCallback);
		_pausedHandle = GCHandle::Alloc(_pausedDelegate);
		IntPtr stoppedIp = Marshal::GetFunctionPointerForDelegate(_pausedDelegate);
		typedef void(__stdcall* PAUSED_CALLBACK) (); // compatible with Core::InputSource::STOPPED_CALLBACK
		GetFFmpegInput()->SetPausedCallback(static_cast<PAUSED_CALLBACK>(stoppedIp.ToPointer()));
	}

	FileInput::~FileInput()
	{
		this->!FileInput();
	}

	FileInput::!FileInput()
	{
		REWRAP_EXCEPTION(
		std::shared_ptr<FFmpeg::FFmpegInput> input = GetFFmpegInput();
		if (!input)
			return;
		input->SetPausedCallback(nullptr);
		_pausedHandle.Free();
		)
	}

	bool FileInput::Seek(TimeSpan time)
	{
		REWRAP_EXCEPTION(return GetFFmpegInput()->Seek(time.Ticks / 10);)
	}

	void FileInput::Play()
	{
		REWRAP_EXCEPTION(GetFFmpegInput()->Play();)
	}

	void FileInput::Pause()
	{
		REWRAP_EXCEPTION(GetFFmpegInput()->Pause();)
	}

	TimeSpan FileInput::AudioDuration::get() { return TimeSpan(GetFFmpegInput()->GetAudioDuration() * 10); }

	TimeSpan FileInput::VideoDuration::get() { return TimeSpan(GetFFmpegInput()->GetVideoDuration() * 10); }

	TimeSpan FileInput::VideoStart::get() { return TimeSpan(GetFFmpegInput()->GetVideoStart() * 10); }

	int FileInput::Width::get() { return GetFFmpegInput()->GetWidth(); }
	
	int FileInput::Height::get() { return GetFFmpegInput()->GetHeight(); }
	
	bool FileInput::IsPlaying::get() { return GetFFmpegInput()->IsPlaying(); }
	
	bool FileInput::IsEof::get() { return GetFFmpegInput()->IsEof(); }
	
	TVPlayR::FieldOrder FileInput::FieldOrder::get() { return GetFFmpegInput()->GetFieldOrder(); }
	
	TVPlayR::Rational FileInput::FrameRate::get() { return TVPlayR::Rational(GetFFmpegInput()->GetFrameRate()); }
	
	int FileInput::AudioChannelCount::get() { return GetFFmpegInput()->GetAudioChannelCount(); }
	
	bool FileInput::HaveAlphaChannel::get() { return GetFFmpegInput()->HaveAlphaChannel(); }

	void FileInput::IsLoop::set(bool isLoop)
	{
		if (isLoop == _isLoop)
			return;
		GetFFmpegInput()->SetIsLoop(isLoop);
		_isLoop = isLoop;
	}
	
	void FileInput::PausedCallback()
	{
		Paused(this, EventArgs::Empty);
	}

	std::shared_ptr<FFmpeg::FFmpegInput> FileInput::GetFFmpegInput()
	{
		return std::dynamic_pointer_cast<FFmpeg::FFmpegInput>(InputBase::GetNativeSource()); 
	}
}