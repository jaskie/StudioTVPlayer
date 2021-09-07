#include "stdafx.h"
#include "DecklinkIterator.h"
#include "DecklinkInfo.h"
#include "DecklinkOutput.h"
#include "DecklinkInput.h"
#include "VideoFormat.h"

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
		Refresh();
	}

	DecklinkOutput^ DecklinkIterator::CreateOutput(DecklinkInfo^ decklink)
	{
		auto native_output = _iterator->CreateOutput(*decklink->GetNativeInfo());
		return gcnew DecklinkOutput(native_output);
	}

	DecklinkInput^ DecklinkIterator::CreateInput(DecklinkInfo^ decklink, VideoFormat^ initialFormat, int audio_channel_count)
	{
		auto native_input = _iterator->CreateInput(*decklink->GetNativeInfo(), initialFormat->GetNativeEnumType(), audio_channel_count);
		return gcnew DecklinkInput(native_input);
	}


}