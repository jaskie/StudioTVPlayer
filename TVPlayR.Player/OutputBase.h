#pragma once
#include "Core/OutputDevice.h"

using namespace System;

namespace TVPlayR {
	public ref class OutputBase abstract
	{
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() abstract;
	};
}