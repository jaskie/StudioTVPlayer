#include "stdafx.h"
#include "DecklinkIterator.h"
#include "DecklinkInfo.h"
#include "DecklinkOutput.h"
#include "DecklinkInput.h"
#include "VideoFormat.h"
#include "Decklink/DecklinkIterator.h"
#include "DecklinkTimecodeSource.h"
#include "DecklinkKeyerType.h"
#include "TimecodeOutputSource.h"

namespace TVPlayR {

	void DecklinkIterator::Refresh()
	{
		REWRAP_EXCEPTION
		(
			int count = static_cast<int>(_iterator->Size());
			_devices = gcnew array<DecklinkInfo^>(count);
			for (int i = 0; i < count; i++)
				_devices[i] = gcnew DecklinkInfo(_iterator->operator[](i));
		)
	}

	static DecklinkIterator::DecklinkIterator()
	{
		REWRAP_EXCEPTION(_iterator = new Decklink::DecklinkIterator();)
		Refresh();
	}

	DecklinkOutput^ DecklinkIterator::CreateOutput(DecklinkInfo^ decklink, DecklinkKeyerType keyer, TimecodeOutputSource timecode_source)
	{
		REWRAP_EXCEPTION
		(
			auto native_output = _iterator->CreateOutput(*decklink->GetNativeInfo(), keyer, timecode_source);
			return gcnew DecklinkOutput(native_output);
		)
	}

	DecklinkInput^ DecklinkIterator::CreateInput(DecklinkInfo^ decklink, VideoFormat^ initialFormat, int audioChannelCount, TVPlayR::DecklinkTimecodeSource timecodeSource, bool captureVideo)
	{
		REWRAP_EXCEPTION
		(
			auto native_input = _iterator->CreateInput(*decklink->GetNativeInfo(), initialFormat->GetNativeEnumType(), audioChannelCount, timecodeSource, captureVideo);
			return gcnew DecklinkInput(native_input, decklink->ModelName);
		)
	}


}