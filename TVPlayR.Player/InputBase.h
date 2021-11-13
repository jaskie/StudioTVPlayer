#pragma once

#include "TimeEventArgs.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR
{
	namespace Core {
		class InputSource;
	}
	public ref class InputBase abstract
	{
	public:
		property String^ Name { String^ get() { return GetName(); } }
		event EventHandler<TimeEventArgs^>^ FramePlayed;
		~InputBase();
		!InputBase();
	internal:
		std::shared_ptr<Core::InputSource> GetNativeSource() { return _nativeSource == nullptr ? nullptr : *_nativeSource; }
	protected:
		InputBase(std::shared_ptr<Core::InputSource> input_source);
		virtual String^ GetName() abstract;
	private:
		const std::shared_ptr<Core::InputSource>* _nativeSource;
		delegate void FramePlayedDelegate(std::int64_t);
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		void FramePlayedCallback(std::int64_t time);
	};
}
