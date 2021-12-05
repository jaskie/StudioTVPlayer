#include "stdafx.h"
#include "DecklinkIterator.h"
#include "DecklinkInfo.h"
#include "DecklinkOutput.h"
#include "DecklinkInput.h"
#include "VideoFormat.h"
#include "Decklink/DecklinkIterator.h"
#include "DecklinkTimecodeSource.h"

namespace TVPlayR {

	void DecklinkIterator::Refresh()
	{
		int count = static_cast<int>(_iterator->Size());
		_devices = gcnew array<DecklinkInfo^>(count);
		for (int i = 0; i < count; i++)
			_devices[i] = gcnew DecklinkInfo(_iterator->operator[](i));
	}

	static DecklinkIterator::DecklinkIterator()
	{
		_iterator = new Decklink::DecklinkIterator();
		Refresh();
	}

	DecklinkOutput^ DecklinkIterator::CreateOutput(DecklinkInfo^ decklink, bool enableInternalKeyer)
	{
		auto native_output = _iterator->CreateOutput(*decklink->GetNativeInfo(), enableInternalKeyer);
		return gcnew DecklinkOutput(native_output);
	}

	DecklinkInput^ DecklinkIterator::CreateInput(DecklinkInfo^ decklink, VideoFormat^ initialFormat, int audioChannelCount, TVPlayR::DecklinkTimecodeSource timecodeSource, bool captureVideo)
	{
		auto native_input = _iterator->CreateInput(*decklink->GetNativeInfo(), initialFormat->GetNativeEnumType(), audioChannelCount, timecodeSource, captureVideo);
		return gcnew DecklinkInput(native_input, decklink->ModelName);
	}


}