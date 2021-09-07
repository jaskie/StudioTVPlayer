#pragma once
#include "Decklink/DecklinkIterator.h"


using namespace System;
using namespace System::Collections::Generic;

namespace TVPlayR {

	ref class DecklinkInfo;
	ref class DecklinkOutput;
	ref class DecklinkInput;
	ref class VideoFormat;

	public ref class DecklinkIterator sealed
	{
	private:
		static DecklinkIterator();
		static array<DecklinkInfo^>^ _devices;
		static Decklink::DecklinkIterator* _iterator = new Decklink::DecklinkIterator();
	public:
		static void Refresh();
		static property array<DecklinkInfo^>^ Devices {
			array<DecklinkInfo^>^ get() { return _devices; }
		}
		static DecklinkOutput^ CreateOutput(DecklinkInfo^ decklink);
		static DecklinkInput^ CreateInput(DecklinkInfo^ decklink, VideoFormat^ initialFormat, int audio_channel_count);
	};

}