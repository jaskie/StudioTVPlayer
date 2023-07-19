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
#include "PreviewSink.h"
#include "FileInput.h"
#include "ClrStringHelper.h"
#include "AudioVolumeEventArgs.h"

namespace TVPlayR {
	void Player::AudioVolumeCallback(std::vector<float>& audio_volume, float coherence)
	{
		array<float>^ result = gcnew array<float>(static_cast<int>(audio_volume.size()));
		if (audio_volume.size())
		{
			pin_ptr<float> dest = &result[0];
			std::memcpy(dest, &audio_volume[0], audio_volume.size() * sizeof(float));
		}
		AudioVolume(this, gcnew AudioVolumeEventArgs(result, coherence));
	}

	Core::Player* CreateNativePlayer(String^ name, TVPlayR::VideoFormat^ videoFormat, TVPlayR::PixelFormat pixelFormat, int audioChannelCount, int sampleRate)
	{
		REWRAP_EXCEPTION(return new Core::Player(ClrStringToStdString(name), videoFormat->GetNativeEnumType(), pixelFormat, audioChannelCount, sampleRate);)
	}

	Core::Player& Player::GetNativePlayer()
	{
		return *_player;
	}

	Player::Player(String^ name, TVPlayR::VideoFormat^ videoFormat, TVPlayR::PixelFormat pixelFormat, int audioChannelCount)
		: _player(CreateNativePlayer(name, videoFormat, pixelFormat, audioChannelCount, 48000))
		, _videoFormat(videoFormat)
		, _pixelFormat(pixelFormat)
	{ 
		_audioVolumeDelegate = gcnew AudioVolumeDelegate(this, &Player::AudioVolumeCallback);
		_audioVolumeHandle = GCHandle::Alloc(_audioVolumeDelegate);
		IntPtr audioVolumeIp = Marshal::GetFunctionPointerForDelegate(_audioVolumeDelegate);
		typedef void(__stdcall* AUDIO_VOLUME_CALLBACK) (std::vector<float>&, float); // compatible with Core::Player::AUDIO_VOLUME_CALLBACK
		_player->SetAudioVolumeCallback(static_cast<AUDIO_VOLUME_CALLBACK>(audioVolumeIp.ToPointer()));
	}

	Player::~Player()
	{
		this->!Player();
	}

	Player::!Player()
	{
		REWRAP_EXCEPTION(
		_player->SetAudioVolumeCallback(nullptr);
		for each (OutputSink ^ output in _outputs)
		{
			std::shared_ptr<Core::OutputSink> native_sink = output->GetNativeSink();
			if (native_sink)
				_player->RemoveOutputSink(native_sink);
		}
		_audioVolumeHandle.Free();
		delete _player;
		)
	}

	void Player::SetFrameClockSource(OutputBase^ output)
	{
		REWRAP_EXCEPTION(_player->SetFrameClockSource(*output->GetNativeDevice());)
	}

	void Player::AddOutputSink(OutputSink^ sink)
	{
		REWRAP_EXCEPTION(
		_player->AddOutputSink(sink->GetNativeSink());
		_outputs->Add(sink);
		)
	}

	void Player::RemoveOutputSink(OutputSink^ sink)
	{
		REWRAP_EXCEPTION(
		if (_outputs->Remove(sink))
			_player->RemoveOutputSink(sink->GetNativeSink());
		)
	}

	void Player::AddOverlay(OverlayBase^ overlay)
	{
		REWRAP_EXCEPTION(
		_player->AddOverlay(overlay->GetNativeObject());
		)
	}

	void Player::Load(InputBase^ file)
	{
		REWRAP_EXCEPTION(
		_player->Load(file->GetNativeSource());
		)
	}

	void Player::LoadNext(InputBase^ file)
	{
		REWRAP_EXCEPTION(
		_player->LoadNext(file->GetNativeSource());
		)
	}

	void Player::Clear()
	{
		REWRAP_EXCEPTION(
		_player->Clear();
		)
	}

	float Player::Volume::get()
	{
		return _volume;
	}

	void Player::Volume::set(float volume)
	{
		if (volume == _volume)
			return;
		REWRAP_EXCEPTION(_player->SetVolume(volume);)
		_volume = volume;
	}

}