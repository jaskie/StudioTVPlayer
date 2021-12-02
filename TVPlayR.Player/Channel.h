#pragma once
#include "Core/Channel.h"
#include "VideoFormat.h"
#include "AudioVolumeEventArgs.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	ref class DecklinkOutput;
	ref class OutputPreview;
	ref class InputBase;
	ref class OutputBase;
	ref class OverlayBase;
	enum class PixelFormat;

	public ref class Channel sealed
	{
	private:
		Core::Channel* const _channel;
		double _volume = 1.0f;
		VideoFormat^ _videoFormat;
		const PixelFormat _pixelFormat;
		delegate void AudioVolumeDelegate(std::vector<double>&);
		AudioVolumeDelegate^ _audioVolumeDelegate;
		GCHandle _audioVolumeHandle;
		System::Collections::Generic::List<OutputBase^>^ _outputs = gcnew System::Collections::Generic::List<OutputBase^>();
		void AudioVolumeCallback(std::vector<double>& audio_volume);
	public:
		Channel(String^ name, VideoFormat^ videoFormat, TVPlayR::PixelFormat pixelFormat, int audioChannelCount);
		~Channel();
		!Channel();
		bool AddOutput(OutputBase^ output, bool setAsClockBase);
		void RemoveOutput(OutputBase^ output);
		void AddOverlay(OverlayBase^ overlay);
		void Load(InputBase^ file);
		void Preload(InputBase^ file);
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
		property TVPlayR::VideoFormat^ VideoFormat { TVPlayR::VideoFormat^ get() { return _videoFormat; }}
		property TVPlayR::PixelFormat PixelFormat { TVPlayR::PixelFormat get() { return _pixelFormat; } }
		event EventHandler<AudioVolumeEventArgs^>^ AudioVolume;
	};
}
