#include "stdafx.h"
#include "DecklinkIterator.h"
#include "DecklinkInfo.h"
#include "DecklinkOutput.h"

namespace TVPlayR {

	array<DecklinkInfo^>^ DecklinkIterator::EnumerateDevices()
	{
		int count = static_cast<int>(_iterator->Size());
		array<DecklinkInfo^>^ devices = gcnew array<DecklinkInfo^>(count);
		for (int i = 0; i < count; i++)
			devices[i] = gcnew DecklinkInfo(_iterator->operator[](i));
		return devices;
	}

	DecklinkOutput^ DecklinkIterator::CreateOutput(DecklinkInfo^ info)
	{
		auto decklink = _iterator->CreateOutput(*info->GetNativeInfo());
		return gcnew DecklinkOutput(decklink);
	}


}