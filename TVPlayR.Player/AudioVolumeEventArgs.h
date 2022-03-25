#pragma once
using namespace System;

namespace TVPlayR {

	public ref class AudioVolumeEventArgs sealed : EventArgs{
	private:
		initonly array<float>^ audio_volume_;
		initonly float coherence_;
	public:
		AudioVolumeEventArgs(array<float>^ audio_volume, float coherence) {
			audio_volume_ = audio_volume;
		}
		property array<float>^ AudioVolume {
			array<float>^ get() { return audio_volume_; }
		}
		property double Coherence {double get() { return coherence_; }}
	};
}
