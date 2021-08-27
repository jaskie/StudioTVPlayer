#pragma once
#include "Decklink/DecklinkIterator.h"


using namespace System;
using namespace System::Collections::Generic;

namespace TVPlayR {

	ref class DecklinkInfo;
	ref class DecklinkOutput;

	public ref class DecklinkIterator sealed
	{
	private:
		static Decklink::DecklinkIterator* _iterator = new Decklink::DecklinkIterator();
	public:
		static array<DecklinkInfo^>^ EnumerateDevices();
		static DecklinkOutput^ CreateOutput(DecklinkInfo^ info);
	};

}