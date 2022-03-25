#pragma once
#include "HardwareAcceleration.h"
#include "InputBase.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {

	value class Rational;
	enum class FieldOrder;
	namespace FFmpeg {
		class FFmpegInput;
	}

	public ref class FileInput : public InputBase
	{
	public:
		FileInput(String^ fileName);
		FileInput(String^ fileName, HardwareAcceleration acceleration, String^ hwDevice);
		~FileInput();
		!FileInput();
		bool Seek(TimeSpan time);
		void Play();
		void Pause();
		property TimeSpan AudioDuration { TimeSpan get(); }
		property TimeSpan VideoDuration { TimeSpan get(); }
		property TimeSpan VideoStart { TimeSpan get(); }
		property String^ FileName { String^ get() { return _fileName; } }
		property int Width { int get(); }
		property int Height { int get(); }
		property bool IsPlaying { bool get(); }
		property bool IsEof { bool get(); }
		property TVPlayR::FieldOrder FieldOrder { TVPlayR::FieldOrder get(); }
		property TVPlayR::Rational FrameRate { TVPlayR::Rational get(); }
		property int AudioChannelCount { int get(); }
		property bool HaveAlphaChannel { bool get(); }
		property bool IsLoop 
		{
			bool get() { return _isLoop; }
			void set(bool isLoop);
		}
		event EventHandler^ Stopped;

	private:
		const HardwareAcceleration _acceleration;
		const String^ _hwDevice;
		String^ _fileName;
		bool _isLoop;

		delegate void StoppedDelegate(void);
		StoppedDelegate^ _stoppedDelegate;
		GCHandle _stoppedHandle;
		void StoppedCallback();
		std::shared_ptr<FFmpeg::FFmpegInput> GetFFmpegInput();
	protected:
		virtual String^ GetName() override { return _fileName; }
	};

}
