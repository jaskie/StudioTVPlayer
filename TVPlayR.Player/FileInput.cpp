#include "stdafx.h"
#include "FileInput.h"
#include "Rational.h"
#include "ClrStringHelper.h"
#include "FFmpeg/ThumbnailFilter.h"
#include "FFmpeg/FFmpegInput.h"
#include "FieldOrder.h"

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

	TimeSpan FileInput::AudioDuration::get() { return TimeSpan(GetFFmpegInput()->GetAudioDuration() * 10); }

	TimeSpan FileInput::VideoDuration::get() { return TimeSpan(GetFFmpegInput()->GetVideoDuration() * 10); }

	TimeSpan FileInput::VideoStart::get() { return TimeSpan(GetFFmpegInput()->GetVideoStart() * 10); }

	int FileInput::Width::get() { return GetFFmpegInput()->GetWidth(); }
	
	int FileInput::Height::get() { return GetFFmpegInput()->GetHeight(); }
	
	bool FileInput::IsPlaying::get() { return GetFFmpegInput()->IsPlaying(); }
	
	bool FileInput::IsEof::get() { return GetFFmpegInput()->IsEof(); }
	
	TVPlayR::FieldOrder FileInput::FieldOrder::get() { return static_cast<TVPlayR::FieldOrder>(GetFFmpegInput()->GetFieldOrder()); }
	
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
	
	void FileInput::StoppedCallback()
	{
		Stopped(this, EventArgs::Empty);
	}

	std::shared_ptr<FFmpeg::FFmpegInput> FileInput::GetFFmpegInput()
	{
		return std::dynamic_pointer_cast<FFmpeg::FFmpegInput>(InputBase::GetNativeSource()); 
	}
}