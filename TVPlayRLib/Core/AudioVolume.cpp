#include "../pch.h"
#include "AudioVolume.h"

namespace TVPlayR {
	namespace Core {
#define ZERO 0x10000UL

AudioVolume::AudioVolume()
	: volume_(ZERO)
	, new_volume_(ZERO)
{
}

void AudioVolume::SetVolume(double volume)
{
	new_volume_ = static_cast<uint32_t>(volume * ZERO);
}


std::vector<double> AudioVolume::ProcessVolume(const std::shared_ptr<AVFrame>& frame, double* coherence)
{
	assert(frame->format == AVSampleFormat::AV_SAMPLE_FMT_S32);
	std::int32_t* samples = reinterpret_cast<int32_t*>(frame->data[0]);
	int samples_count = frame->nb_samples * frame->channels;
	std::vector<std::int64_t> peak_volume_int64(frame->channels, 0LL);
	for (int sample = 0; sample < samples_count; sample++)
	{
		if (volume_ != new_volume_ && sample > 0 && (samples[sample] * samples[sample - 1]) < 0) // sign of sample was changed, probably near zero, we can now adjust the volume
			volume_ = new_volume_;
		samples[sample] = av_clipl_int32((((std::int64_t)samples[sample] * volume_ / ZERO)));
		if (std::abs(samples[sample]) > peak_volume_int64[sample % frame->channels])
			peak_volume_int64[sample % frame->channels] = std::abs(samples[sample]);
	}
	if (volume_ != new_volume_)
		volume_ = new_volume_; // in case when volume change failed within one frame period
	std::vector<double> peak_volume_double(frame->channels);
	std::transform(peak_volume_int64.begin(), peak_volume_int64.end(), peak_volume_double.begin(), [](std::int64_t volume) { return static_cast<double>(volume) / (std::numeric_limits<int32_t>().max)(); });
	if (coherence)
	{
		//TODO: calculate the coherence
		*coherence = 0.0;
	}
	return peak_volume_double;
}

}}