#pragma once
using namespace System;

namespace TVPlayR {

	public ref class AudioVolumeEventArgs sealed : EventArgs{
	private:
		initonly array<double>^ audio_volume_;
		initonly double coherence_;
	public:
		AudioVolumeEventArgs(array<double>^ audio_volume, double coherence) {
			audio_volume_ = audio_volume;
		}
		property array<double>^ AudioVolume {
			array<double>^ get() { return audio_volume_; }
		}
		property double Coherence {double get() { return coherence_; }}
	};
}
