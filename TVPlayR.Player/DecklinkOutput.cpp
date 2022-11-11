#include "stdafx.h"
#include "DecklinkOutput.h"
#include "OverlayBase.h"
#include "Decklink/DecklinkOutput.h"
#include "Player.h"
#include "Core/Player.h"
#include "Core/VideoFormat.h"


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
		REWRAP_EXCEPTION((*_decklink)->AddOverlay(overlay->GetNativeObject());)
	}

	void DecklinkOutput::RemoveOverlay(OverlayBase^ overlay)
	{
		if (!_decklink)
			return;
		REWRAP_EXCEPTION((*_decklink)->RemoveOverlay(overlay->GetNativeObject());)
	}

	void DecklinkOutput::InitializeFor(Player^ player)
	{
		Core::Player& native_player = player->GetNativePlayer();
		REWRAP_EXCEPTION((*_decklink)->Initialize(native_player.Format().type(), native_player.PixelFormat(), native_player.AudioChannelsCount(), native_player.AudioSampleRate());)
	}

	DecklinkOutput::~DecklinkOutput()
	{
		this->!DecklinkOutput();
	}

	DecklinkOutput::!DecklinkOutput()
	{
		if (!_decklink)
			return;
		REWRAP_EXCEPTION(delete _decklink;)
		_decklink = nullptr;
	}
}