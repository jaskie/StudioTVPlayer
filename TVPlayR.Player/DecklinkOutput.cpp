#include "stdafx.h"
#include "DecklinkOutput.h"
#include "Decklink/DecklinkOutput.h"

namespace TVPlayR {
	std::shared_ptr<Core::OutputDevice> DecklinkOutput::GetNativeDevice()
	{
		return _decklink == nullptr ? nullptr : *_decklink;
	}
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