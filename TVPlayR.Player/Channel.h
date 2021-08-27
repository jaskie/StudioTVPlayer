#pragma once
#include "Core/Channel.h"
#include "VideoFormat.h"
#include "PixelFormat.h"
#include "AudioVolumeEventArgs.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	ref class DecklinkOutput;
	ref class PreviewOutput;
	ref class InputFile;
	ref class OutputBase;

	public ref class Channel sealed
	{
	private:
		Core::Channel* const _channel;
		double _volume = 1.0f;
		delegate void AudioVolumeDelegate(std::vector<double>);
		AudioVolumeDelegate^ _audioVolumeDelegate;
		GCHandle _audioVolumeHandle;
		System::Collections::Generic::List<OutputBase^>^ _outputs = gcnew System::Collections::Generic::List<OutputBase^>();
		void AudioVolumeCallback(std::vector<double> audio_volume);
	public:
		Channel(String^ name, VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount);
		~Channel();
		!Channel();
		bool AddOutput(OutputBase^ device, bool setAsClockBase);
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
