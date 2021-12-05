#pragma once

using namespace System;

namespace TVPlayR {
	namespace Core {
		class OverlayBase;
	}

	public ref class OverlayBase abstract
	{
	internal:
		virtual std::shared_ptr<Core::OverlayBase> GetNativeObject() abstract;
	};
}