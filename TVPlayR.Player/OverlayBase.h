#pragma once
#include "Core/OverlayBase.h"

using namespace System;

namespace TVPlayR {
	public ref class OverlayBase abstract
	{
	internal:
		virtual std::shared_ptr<Core::OverlayBase> GetNativeObject() abstract;
	};
}