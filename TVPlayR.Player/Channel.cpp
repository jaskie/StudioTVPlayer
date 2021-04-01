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
		array<double>^ result = gcnew array<double>(static_cast<int>(audio_volume.size()));
		if (audio_volume.size())
		{
			pin_ptr<double> dest = &result[0];
			std::memcpy(dest, &audio_volume[0], audio_volume.size() * sizeof(double));
		}
		AudioVolume(this, gcnew AudioVolumeEventArgs(result));
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