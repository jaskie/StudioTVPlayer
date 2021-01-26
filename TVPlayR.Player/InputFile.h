#pragma once
#include "FFMpeg/FFMpegInputSource.h"
#include "TimeEventArgs.h"
#include "HardwareAcceleration.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;


namespace TVPlayR {

	public ref class InputFile
	{
	public:
		InputFile(String^ fileName, int audioChannelCount);
		InputFile(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice, int audioChannelCount);
		~InputFile();
		!InputFile();
		bool Seek(TimeSpan time);
		void Play();
		void Pause();
		Bitmap^ GetThumbnail(int height);
		BitmapSource^ GetBitmapSource(int height);
		property TimeSpan AudioDuration 
		{
			TimeSpan get() { return TimeSpan((*_nativeSource)->GetAudioDuration() * 10); }
		}
		property TimeSpan VideoDuration 
		{
			TimeSpan get() { return TimeSpan((*_nativeSource)->GetVideoDuration() * 10); }
		}
		property String^ FileName
		{
			String^ get() { return _fileName; }
		}

		property bool IsPlaying
		{
			bool get() { return (*_nativeSource)->IsPlaying(); }
		}

		delegate void FramePlayedDelegate(int64_t);
		delegate void StoppedDelegate(void);
		event EventHandler<TimeEventArgs^>^ FramePlayed;
		event EventHandler^ Stopped;
	private:
		const HardwareAcceleration _acceleration;
		const String^ _hwDevice;
		std::shared_ptr<FFmpeg::FFmpegInputSource> * _nativeSource;
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		StoppedDelegate^ _stoppedDelegate;
		GCHandle _stoppedHandle;
		String^ _fileName;
		void FramePlayedCallback(int64_t time);
		void StoppedCallback();
	internal:
		std::shared_ptr<FFmpeg::FFmpegInputSource>& GetNativeSource() { return *_nativeSource; }
	};

}
