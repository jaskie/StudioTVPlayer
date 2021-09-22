#pragma once
namespace TVPlayR
{
	namespace Core {
		class InputSource;
	}
	public ref class InputBase abstract
	{
	internal:
		virtual std::shared_ptr<Core::InputSource> GetNativeSource() abstract;
	};
}
