#include "stdafx.h"
#include "DecklinkInput.h"
#include "Decklink/DecklinkInput.h"
#include "InputPreview.h"

namespace TVPlayR {

	DecklinkInput::DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink)
		: _decklink(new std::shared_ptr<Decklink::DecklinkInput>(decklink))
	{
	}

	void DecklinkInput::AddPreview(InputPreview^ preview)
	{
		(*_decklink)->AddPreview(preview->GetNative());
	}

	void DecklinkInput::RemovePreview(InputPreview^ preview)
	{
		(*_decklink)->RemovePreview(preview->GetNative());
	}

	DecklinkInput::~DecklinkInput()
	{
		this->!DecklinkInput();
	}

	DecklinkInput::!DecklinkInput()
	{
		if (!_decklink)
			return;
		delete _decklink;
		_decklink = nullptr;
	}
}