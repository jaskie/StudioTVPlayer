#pragma once

namespace TVPlayR {
	namespace Decklink {
		class DecklinkInput;
	}
	public enum class DecklinkTimecodeSource {
		None,
		StreamTime,
		RP188Any
	};

	public ref class DecklinkInput
	{
	private:
		const std::shared_ptr<Decklink::DecklinkInput>* _decklink;
	internal:
		DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink);
		std::shared_ptr<Decklink::DecklinkInput> GetNativeDevice() { return *_decklink; }
	public:
		~DecklinkInput();
		!DecklinkInput();
	};

}
