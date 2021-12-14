#include "stdafx.h"
#include "Player.h"
#include "FieldOrder.h"
#include "VideoFormat.h"
#include "Core/Player.h"
#include "Core/OutputDevice.h"
#include "PixelFormat.h"
#include "OutputBase.h"
#include "OverlayBase.h"
#include "DecklinkOutput.h"
#include "OutputPreview.h"
#include "FileInput.h"
#include "ClrStringHelper.h"
#include "AudioVolumeEventArgs.h"

namespace TVPlayR {
	void Player::AudioVolumeCallback(std::vector<double>& audio_volume, double coherence)
	{
		array<double>^ result = gcnew array<double>(static_cast<int>(audio_volume.size()));
		if (audio_volume.size())
		{
			pin_ptr<double> dest = &result[0];
			std::memcpy(dest, &audio_volume[0], audio_volume.size() * sizeof(double));
		}
		AudioVolume(this, gcnew AudioVolumeEventArgs(result, coherence));
	}

	Player::Player(String^ name, TVPlayR::VideoFormat^ videoFormat, TVPlayR::PixelFormat pixelFormat, int audioChannelCount)
		: _player(new Core::Player(ClrStringToStdString(name), videoFormat->GetNativeEnumType(), pixelFormat, audioChannelCount, 48000))
		, _videoFormat(videoFormat)
		, _pixelFormat(pixelFormat)
	{ 
		_audioVolumeDelegate = gcnew AudioVolumeDelegate(this, &Player::AudioVolumeCallback);
		_audioVolumeHandle = GCHandle::Alloc(_audioVolumeDelegate);
		IntPtr audioVolumeIp = Marshal::GetFunctionPointerForDelegate(_audioVolumeDelegate);
		typedef void(__stdcall* AUDIO_VOLUME_CALLBACK) (std::vector<double>&, double); // compatible with Core::Player::AUDIO_VOLUME_CALLBACK
		_player->SetAudioVolumeCallback(static_cast<AUDIO_VOLUME_CALLBACK>(audioVolumeIp.ToPointer()));
	}

	Player::~Player()
	{
		this->!Player();
	}

	Player::!Player()
	{
		_player->SetAudioVolumeCallback(nullptr);
		for each (OutputBase ^ output in _outputs)
		{
			std::shared_ptr<Core::OutputDevice> native_ouptut = output->GetNativeDevice();
			if (native_ouptut)
				_player->RemoveOutput(native_ouptut);
		}
		_audioVolumeHandle.Free();
		delete _player;
	}

	bool Player::AddOutput(OutputBase^ output, bool setAsClockBase)
	{
		if (output == nullptr)
			return false;
		if (setAsClockBase)
			_player->SetFrameClock(output->GetNativeDevice());
		if (!_player->AddOutput(output->GetNativeDevice()))
			return false;
		_outputs->Add(output);
		return true;
	}

	void Player::RemoveOutput(OutputBase^ output)
	{
		if (_outputs->Remove(output))
			_player->RemoveOutput(output->GetNativeDevice());
	}

	void Player::AddOverlay(OverlayBase^ overlay)
	{
		_player->AddOverlay(overlay->GetNativeObject());
	}

	void Player::Load(InputBase^ file)
	{
		_player->Load(file->GetNativeSource());
	}

	void Player::Preload(InputBase^ file)
	{
		_player->Preload(file->GetNativeSource());
	}

	void Player::Clear()
	{
		_player->Clear();
	}

	double Player::Volume::get()
	{
		return _volume;
	}

	void Player::Volume::set(double volume)
	{
		if (volume == _volume)
			return;
		_player->SetVolume(volume);
		_volume = volume;
	}

}