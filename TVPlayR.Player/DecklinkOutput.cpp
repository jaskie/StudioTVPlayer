#include "stdafx.h"
#include "DecklinkOutput.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"



namespace TVPlayR {

	DecklinkOutput::DecklinkOutput(std::shared_ptr<Decklink::DecklinkOutput>& decklink)
		: _decklink(new std::shared_ptr<Decklink::DecklinkOutput>(decklink))
	{
	}

	DecklinkOutput::~DecklinkOutput()
	{
		this->!DecklinkOutput();
	}

	DecklinkOutput::!DecklinkOutput()
	{
		if (!_decklink)
			return;
		delete _decklink;
		_decklink = nullptr;
	}
}