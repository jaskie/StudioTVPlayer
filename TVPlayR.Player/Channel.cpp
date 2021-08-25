#include "stdafx.h"
#include "Channel.h"
#include "Core/Channel.h"
#include "Decklink/Iterator.h"
#include "OutputBase.h"
#include "DecklinkOutput.h"
#include "PreviewOutput.h"
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
		_channel->SetAudioVolumeCallback(nullptr);
		for each (OutputBase^ output in _outputs)
			_channel->RemoveOutput(output->GetNativeDevice());
		delete _channel;
	}

	bool Channel::AddOutput(OutputBase ^ device, bool setAsClockBase)
	{
		if (setAsClockBase)
			_channel->SetFrameClock(device->GetNativeDevice());
		if (!_channel->AddOutput(device->GetNativeDevice()))
			return false;
		_outputs->Add(device);
		return true;
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