#pragma once
#include "FFmpeg/FFmpegFileInfo.h"
#include "HardwareAcceleration.h"
#include "FieldOrder.h"
#include "Rational.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {

	public ref class FileInfo
	{
	public:
		FileInfo(String^ fileName);
		FileInfo(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice);
		~FileInfo();
		!FileInfo();
		Bitmap^ GetThumbnail(TimeSpan time, int width, int height);
		BitmapSource^ GetBitmapSource(TimeSpan time, int width, int height);
		property TimeSpan AudioDuration { TimeSpan get() { return TimeSpan((*_nativeSource)->GetAudioDuration() * 10); } }
		property TimeSpan VideoDuration { TimeSpan get() { return TimeSpan((*_nativeSource)->GetVideoDuration() * 10); } }
		property TimeSpan VideoStart { TimeSpan get() { return TimeSpan((*_nativeSource)->GetVideoStart() * 10); } }
		property String^ FileName { String^ get() { return _fileName; } }
		property int Width { int get() { return (*_nativeSource)->GetWidth(); } }
		property int Height { int get() { return (*_nativeSource)->GetHeight(); } }
		property TVPlayR::FieldOrder FieldOrder { TVPlayR::FieldOrder get() { return static_cast<TVPlayR::FieldOrder>((*_nativeSource)->GetFieldOrder()); } }
		property TVPlayR::Rational FrameRate { TVPlayR::Rational get() { return TVPlayR::Rational((*_nativeSource)->GetFrameRate()); } }
		property int AudioChannelCount { int get() { return (*_nativeSource)->GetAudioChannelCount(); }}
		property bool HaveAlphaChannel { bool get() { return (*_nativeSource)->HaveAlphaChannel(); }}
		property bool IsStream {bool get() { return (*_nativeSource)->IsStream(); }}

	private:
		const HardwareAcceleration _acceleration;
		const String^ _hwDevice;
		const std::shared_ptr<FFmpeg::FFmpegFileInfo>* _nativeSource;
		String^ _fileName;

	internal:
		const std::shared_ptr<FFmpeg::FFmpegFileInfo>& GetNativeSource() { return *_nativeSource; }
	};

}
