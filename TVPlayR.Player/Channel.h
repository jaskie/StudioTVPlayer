#pragma once
#include "Core/Channel.h"
#include "VideoFormat.h"
#include "PixelFormat.h"
#include "AudioVolumeEventArgs.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	ref class DecklinkDevice;
	ref class PreviewDevice;
	ref class InputFile;

	public ref class Channel
	{
	private:
		Core::Channel* _channel;
		double _volume = 1.0f;
		delegate void AudioVolumeDelegate(std::vector<double>);
		AudioVolumeDelegate^ _audioVolumeDelegate;
		GCHandle _audioVolumeHandle;
		void AudioVolumeCallback(std::vector<double> audio_volume);
	public:
		Channel(VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount);
		~Channel();
		!Channel();
		bool AddOutput(DecklinkDevice^ device);
		bool AddPreview(PreviewDevice^ preview);
		void Load(InputFile^ file);
		void Preload(InputFile^ file);
		void Clear();
		property double Volume
		{
			double get() { return _volume; }
			void set(double volume) 
			{
				if (volume == _volume)
					return;
				_channel->SetVolume(volume);
				_volume = volume;
			}
		}
		event EventHandler<AudioVolumeEventArgs^>^ AudioVolume;
	};
}
