#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace TVPlayR {
	ref class DecklinkInfo;
	ref class DecklinkOutput;
	ref class DecklinkInput;
	ref class VideoFormat;
	enum class DecklinkTimecodeSource;
	enum class DecklinkKeyer;

	namespace Decklink {
		class DecklinkIterator;
	}


	public ref class DecklinkIterator sealed
	{
	private:
		static DecklinkIterator();
		static array<DecklinkInfo^>^ _devices;
		static Decklink::DecklinkIterator* _iterator;
	public:
		static void Refresh();
		static property array<DecklinkInfo^>^ Devices {
			array<DecklinkInfo^>^ get() { return _devices; }
		}
		static DecklinkOutput^ CreateOutput(DecklinkInfo^ decklink, DecklinkKeyer keyer);
		static DecklinkInput^ CreateInput(DecklinkInfo^ decklink, VideoFormat^ initialFormat, int audioChannelCount, TVPlayR::DecklinkTimecodeSource timecodeSource, bool captureVideo);
	};
}
