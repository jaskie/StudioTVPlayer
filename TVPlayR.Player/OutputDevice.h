#pragma once
#include "Core/OutputDevice.h"

using namespace System;

namespace TVPlayR {
	public ref class OutputDevice abstract
	{
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() abstract;
	};
}