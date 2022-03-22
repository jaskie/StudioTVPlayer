#include "stdafx.h"
#include "DecklinkOutput.h"
#include "OverlayBase.h"
#include "Decklink/DecklinkOutput.h"

namespace TVPlayR {
	std::shared_ptr<Core::OutputDevice> DecklinkOutput::GetNativeDevice()
	{
		return _decklink == nullptr ? nullptr : *_decklink;
	}
	std::shared_ptr<Core::OutputSink> DecklinkOutput::GetNativeSink()
	{
		return _decklink == nullptr ? nullptr : *_decklink;
	}

	DecklinkOutput::DecklinkOutput(std::shared_ptr<Decklink::DecklinkOutput>& decklink)
		: _decklink(new std::shared_ptr<Decklink::DecklinkOutput>(decklink))
	{
	}

	void DecklinkOutput::AddOverlay(OverlayBase^ overlay)
	{
		if (!_decklink)
			return;
		(*_decklink)->AddOverlay(overlay->GetNativeObject());
	}

	void DecklinkOutput::RemoveOverlay(OverlayBase^ overlay)
	{
		if (!_decklink)
			return;
		(*_decklink)->RemoveOverlay(overlay->GetNativeObject());
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