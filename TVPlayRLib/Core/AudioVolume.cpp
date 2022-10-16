#include "../pch.h"
#include "AudioVolume.h"

namespace TVPlayR {
	namespace Core {

AudioVolume::AudioVolume()
	: volume_(1.0)
	, new_volume_(1.0)
{
}

void AudioVolume::SetVolume(float volume)
{
	new_volume_ = volume;
}


std::vector<float> AudioVolume::ProcessVolume(const std::shared_ptr<AVFrame>& frame, float* coherence)
{
	assert(frame->format == AVSampleFormat::AV_SAMPLE_FMT_FLT);
	float* samples = reinterpret_cast<float*>(frame->data[0]);
	int channels = frame->ch_layout.nb_channels;
	int samples_count = frame->nb_samples * channels;
	std::vector<float> peak_volume(channels, 0LL);
	for (int sample = 0; sample < samples_count; sample++)
	{
		if (volume_ != new_volume_ && sample > 0 && (samples[sample] * samples[sample - 1]) < 0) // sign of sample was changed, probably near zero, we can now adjust the volume
			volume_ = new_volume_;
		samples[sample] = samples[sample] * volume_;
		if (std::abs(samples[sample]) > peak_volume[sample % channels])
			peak_volume[sample % channels] = std::abs(samples[sample]);
	}
	if (volume_ != new_volume_)
		volume_ = new_volume_; // in case when volume change failed within one frame period
	if (coherence)
	{
		//TODO: calculate the coherence
		*coherence = 0.0;
	}
	return peak_volume;
}

}}