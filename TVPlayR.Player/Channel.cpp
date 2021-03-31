#include "stdafx.h"
#include "Channel.h"
#include "Core/Channel.h"
#include "Decklink/Iterator.h"
#include "DecklinkDevice.h"
#include "PreviewDevice.h"
#include "InputFile.h"


namespace TVPlayR {
	void Channel::AudioVolumeCallback(std::vector<double>audio_volume)
	{
		AudioVolume(this, gcnew AudioVolumeEventArgs(audio_volume));
	}

	Channel::Channel(VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount)
		: _channel(new Core::Channel(videoFormat->GetNativeEnumType(), static_cast<Core::PixelFormat>(pixelFormat), audioChannelCount))
	{ 
		_audioVolumeDelegate = gcnew AudioVolumeDelegate(this, &Channel::AudioVolumeCallback);
		_audioVolumeHandle = GCHandle::Alloc(_audioVolumeDelegate);
		IntPtr audioVolumeIp = Marshal::GetFunctionPointerForDelegate(_audioVolumeDelegate);
		_channel->SetAudioVolumeCallback(static_cast<Core::Channel::AUDIO_VOLUME_CALLBACK>(audioVolumeIp.ToPointer()));

	}

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

	bool Channel::AddPreview(PreviewDevice^ preview)
	{
		return _channel->AddOutput(preview->GetNativeDevice());
	}

	void Channel::Load(InputFile ^ file)
	{
		_channel->Load(file->GetNativeSource());
	}

	void Channel::Preload(InputFile^ file)
	{
		_channel->Preload(file->GetNativeSource());
	}

	void Channel::Clear()
	{
		_channel->Clear();
	}

}