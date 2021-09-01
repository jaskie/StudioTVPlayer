#pragma once
#include "FFMpeg/FFMpegInputSource.h"
#include "TimeEventArgs.h"
#include "HardwareAcceleration.h"
#include "FieldOrder.h"
#include "Rational.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {

	public ref class FileInput
	{
	public:
		FileInput(String^ fileName, int audioChannelCount);
		FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice, int audioChannelCount);
		~FileInput();
		!FileInput();
		bool Seek(TimeSpan time);
		void Play();
		void Pause();
		Bitmap^ GetThumbnail(TimeSpan time, int width, int height);
		BitmapSource^ GetBitmapSource(TimeSpan time, int width, int height);
		property TimeSpan AudioDuration 
		{
			TimeSpan get() { return TimeSpan((*_nativeSource)->GetAudioDuration() * 10); }
		}
		property TimeSpan VideoDuration 
		{
			TimeSpan get() { return TimeSpan((*_nativeSource)->GetVideoDuration() * 10); }
		}
		property TimeSpan VideoStart
		{
			TimeSpan get() { return TimeSpan((*_nativeSource)->GetVideoStart() * 10); }
		}

		property String^ FileName
		{
			String^ get() { return _fileName; }
		}

		property int Width
		{
			int get() { return (*_nativeSource)->GetWidth(); }
		}

		property int Height
		{
			int get() { return (*_nativeSource)->GetHeight(); }
		}

		property bool IsPlaying
		{
			bool get() { return (*_nativeSource)->IsPlaying(); }
		}

		property bool IsEof
		{
			bool get() { return (*_nativeSource)->IsEof(); }
		}

		property TVPlayR::FieldOrder FieldOrder
		{
			TVPlayR::FieldOrder get() { return static_cast<TVPlayR::FieldOrder>((*_nativeSource)->GetFieldOrder()); }
		}

		property TVPlayR::Rational FrameRate
		{
			TVPlayR::Rational get() { return TVPlayR::Rational((*_nativeSource)->GetFrameRate()); }
		}

		property int AudioChannelCount { int get() { return (*_nativeSource)->GetAudioChannelCount(); }}

		property bool HaveAlphaChannel { bool get() { return (*_nativeSource)->HaveAlphaChannel(); }}

		property bool IsLoop 
		{
			bool get() { return _isLoop; }
			void set(bool isLoop)
			{
				if (isLoop == _isLoop)
					return;
				(*_nativeSource)->SetIsLoop(isLoop);
				_isLoop = isLoop;
			}
		}

		event EventHandler<TimeEventArgs^>^ FramePlayed;
		event EventHandler^ Stopped;
	private:
		const HardwareAcceleration _acceleration;
		const String^ _hwDevice;
		const std::shared_ptr<FFmpeg::FFmpegInputSource> * _nativeSource;
		String^ _fileName;
		bool _isLoop;

		delegate void FramePlayedDelegate(int64_t);
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		void FramePlayedCallback(int64_t time);

		delegate void StoppedDelegate(void);
		StoppedDelegate^ _stoppedDelegate;
		GCHandle _stoppedHandle;
		void StoppedCallback();
	internal:
		const std::shared_ptr<FFmpeg::FFmpegInputSource>& GetNativeSource() { return *_nativeSource; }
	};

}
