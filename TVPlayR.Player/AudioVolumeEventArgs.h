#pragma once
using namespace System;

namespace TVPlayR {

	public ref class AudioVolumeEventArgs sealed : EventArgs{
	private:
		initonly array<double>^ audio_volume_;
	public:
		AudioVolumeEventArgs(array<double>^ audio_volume) {
			audio_volume_ = audio_volume;
		}
		property array<double>^ AudioVolume {
			array<double>^ get() { return audio_volume_; }
		}
	};
}
