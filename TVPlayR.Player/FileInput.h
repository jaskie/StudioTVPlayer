#pragma once
#include "FFmpeg/FFmpegInput.h"
#include "HardwareAcceleration.h"
#include "FieldOrder.h"
#include "Rational.h"
#include "InputBase.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {

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
		property TimeSpan AudioDuration { TimeSpan get() { return TimeSpan(GetFFmpegInput()->GetAudioDuration() * 10); } }
		property TimeSpan VideoDuration { TimeSpan get() { return TimeSpan(GetFFmpegInput()->GetVideoDuration() * 10); } }
		property TimeSpan VideoStart { TimeSpan get() { return TimeSpan(GetFFmpegInput()->GetVideoStart() * 10); } }
		property String^ FileName { String^ get() { return _fileName; } }
		property int Width { int get() { return GetFFmpegInput()->GetWidth(); } }
		property int Height { int get() { return GetFFmpegInput()->GetHeight(); } }
		property bool IsPlaying { bool get() { return GetFFmpegInput()->IsPlaying(); } }
		property bool IsEof { bool get() { return GetFFmpegInput()->IsEof(); } }
		property TVPlayR::FieldOrder FieldOrder { TVPlayR::FieldOrder get() { return static_cast<TVPlayR::FieldOrder>(GetFFmpegInput()->GetFieldOrder()); } }
		property TVPlayR::Rational FrameRate { TVPlayR::Rational get() { return TVPlayR::Rational(GetFFmpegInput()->GetFrameRate()); } }
		property int AudioChannelCount { int get() { return GetFFmpegInput()->GetAudioChannelCount(); }}
		property bool HaveAlphaChannel { bool get() { return GetFFmpegInput()->HaveAlphaChannel(); }}
		property bool IsLoop 
		{
			bool get() { return _isLoop; }
			void set(bool isLoop)
			{
				if (isLoop == _isLoop)
					return;
				GetFFmpegInput()->SetIsLoop(isLoop);
				_isLoop = isLoop;
			}
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
		std::shared_ptr<FFmpeg::FFmpegInput> GetFFmpegInput() { return std::dynamic_pointer_cast<FFmpeg::FFmpegInput>(InputBase::GetNativeSource()); }
	protected:
		virtual String^ GetName() override { return _fileName; }
	};

}
