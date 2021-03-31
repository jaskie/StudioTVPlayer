#pragma once
using namespace System;

namespace TVPlayR {

	public ref class AudioVolumeEventArgs: EventArgs{
	private:
		initonly double audio_volume_;
	public:
		AudioVolumeEventArgs(double[]^ audio_volume) {
			audio_volume_ = audio_volume;
		}
		property double AudioVolume {
			double get() { return audio_volume_; }
		}
	};

}
