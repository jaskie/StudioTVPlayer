#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR
{
	ref class TimeEventArgs;
	ref class OutputSink;
	namespace Core {
		class InputSource;
		struct FrameTimeInfo;
	}
	public ref class InputBase abstract
	{
	public:
		property String^ Name { String^ get() { return GetName(); } }
		event EventHandler<TimeEventArgs^>^ FramePlayed;
		~InputBase();
		!InputBase();
		void AddOutputSink(OutputSink^ output);
		void RemoveOutputSink(OutputSink^ output);
	internal:
		std::shared_ptr<Core::InputSource> GetNativeSource() { return _nativeSource == nullptr ? nullptr : *_nativeSource; }
	protected:
		InputBase(std::shared_ptr<Core::InputSource>& input_source);
		virtual String^ GetName() abstract;
	private:
		const std::shared_ptr<Core::InputSource>* _nativeSource;
		delegate void FramePlayedDelegate(Core::FrameTimeInfo& time_info);
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		void FramePlayedCallback(Core::FrameTimeInfo& time_info);
	};
}
