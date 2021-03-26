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
		Bitmap^ GetThumbnail(TimeSpan time, int height);
		BitmapSource^ GetBitmapSource(TimeSpan time, int height);
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
			TVPlayR::FieldOrder get() 
			{
				auto internalFieldOrder = (*_nativeSource)->GetFieldOrder();
				switch (internalFieldOrder)
				{
				case AVFieldOrder::AV_FIELD_TT:
				case AVFieldOrder::AV_FIELD_TB:
					return TVPlayR::FieldOrder::TopFieldFirst;
				case AVFieldOrder::AV_FIELD_BB:
				case AVFieldOrder::AV_FIELD_BT:
					return TVPlayR::FieldOrder::BottomFieldFirst;
				default:
					return TVPlayR::FieldOrder::Progressive;
				}
			}
		}

		property TVPlayR::Rational FrameRate
		{
			TVPlayR::Rational get() { return TVPlayR::Rational((*_nativeSource)->GetFrameRate()); }
		}

		property int AudioChannelCount { int get() { return (*_nativeSource)->GetAudioChannelCount(); }}

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
