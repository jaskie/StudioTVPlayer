#pragma once

using namespace System;

namespace TVPlayR
{
	namespace Core {
		class InputSource;
	}
	public ref class InputBase abstract
	{
	public:
		property String^ Name { String^ get() { return GetName(); } }
	internal:
		virtual std::shared_ptr<Core::InputSource> GetNativeSource() abstract;
	protected:
		virtual String^ GetName() abstract;
	};
}
