#pragma once
#include "Core/OutputDevice.h"

namespace TVPlayR {

	public ref class OutputDevice abstract
	{
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() abstract;
	};

}
