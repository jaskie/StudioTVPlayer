#pragma once

using namespace System;

namespace TVPlayR {
	namespace Core {
		class OutputDevice;
	}
	public ref class OutputBase abstract
	{
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() abstract;
	};
}