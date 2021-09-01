#include "stdafx.h"
#include "DecklinkInput.h"
#include "Decklink/DecklinkInput.h"

namespace TVPlayR {


	DecklinkInput::DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink)
		: _decklink(new std::shared_ptr<Decklink::DecklinkInput>(decklink))
	{
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