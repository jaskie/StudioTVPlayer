#pragma once
#include "InputBase.h"
#include "Decklink/DecklinkInput.h"

namespace TVPlayR {
	ref class InputPreview;

	namespace Decklink {
		class DecklinkInput;
	}
	public enum class DecklinkTimecodeSource {
		None,
		RP188Any,
		VITC
	};

	public ref class DecklinkInput : public InputBase
	{
	private:
		const std::shared_ptr<Decklink::DecklinkInput>* _decklink;
	internal:
		DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink);
		virtual std::shared_ptr<Core::InputSource> GetNativeSource() override { return _decklink ? *_decklink : nullptr; }
	public:
		void AddPreview(InputPreview^ preview);
		void RemovePreview(InputPreview^ preview);
		~DecklinkInput();
		!DecklinkInput();
	};

}
