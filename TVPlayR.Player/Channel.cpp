#include "stdafx.h"
#include "Channel.h"
#include "Core/Channel.h"
#include "Decklink/Iterator.h"

namespace TVPlayR {
	
	Channel::Channel(VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount)
		: _channel(new Core::Channel(videoFormat->GetNativeEnumType(), static_cast<Core::PixelFormat>(pixelFormat), audioChannelCount))
	{ }

	Channel::Channel(int formatId, PixelFormat pixelFormat, int audioChannelCount)
		: _channel(new Core::Channel(static_cast<Core::VideoFormat::Type>(formatId), static_cast<Core::PixelFormat>(pixelFormat), audioChannelCount))
	{ }

	Channel::~Channel()
	{
		this->!Channel();
	}

	Channel::!Channel()
	{
		if (!_channel)
			return;
		delete _channel;
		_channel = nullptr;
	}

	bool Channel::AddOutput(DecklinkDevice ^ device)
	{
		if (!_channel->AddOutput(device->GetNativeDevice()))
			return false;
		_channel->SetFrameClock(device->GetNativeDevice());
		return true;
	}

	void Channel::Load(InputFile ^ file)
	{
		_channel->Load(std::dynamic_pointer_cast<Core::InputSource>(file->GetNativeSource()));
	}

	void Channel::Clear()
	{
		_channel->Clear();
	}


}