#pragma once
#include "HardwareAcceleration.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {

	value class Rational;
	enum class FieldOrder;
	namespace FFmpeg {
		class FFmpegFileInfo;
	}

	public ref class FileInfo
	{
	public:
		FileInfo(String^ fileName);
		FileInfo(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice);
		~FileInfo();
		!FileInfo();
		Bitmap^ GetThumbnail(TimeSpan time, int width, int height);
		BitmapSource^ GetBitmapSource(TimeSpan time, int width, int height);
		property TimeSpan AudioDuration { TimeSpan get(); }
		property TimeSpan VideoDuration { TimeSpan get(); }
		property TimeSpan VideoStart { TimeSpan get(); }
		property String^ FileName { String^ get() { return _fileName; } }
		property int Width { int get(); }
		property int Height { int get(); }
		property TVPlayR::FieldOrder FieldOrder { TVPlayR::FieldOrder get(); }
		property TVPlayR::Rational FrameRate { TVPlayR::Rational get(); }
		property int AudioChannelCount { int get(); }
		property bool HaveAlphaChannel { bool get(); }
		property bool IsStream { bool get(); }

	private:
		const HardwareAcceleration _acceleration;
		const String^ _hwDevice;
		const std::shared_ptr<FFmpeg::FFmpegFileInfo>* _nativeSource;
		String^ _fileName;

	internal:
		const std::shared_ptr<FFmpeg::FFmpegFileInfo>& GetNativeSource() { return *_nativeSource; }
	};

}
