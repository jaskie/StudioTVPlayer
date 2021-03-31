#include "../pch.h"
#include "AudioVolume.h"

namespace TVPlayR {
	namespace Core {
#define ZERO 0x10000UL

AudioVolume::AudioVolume()
	: old_volume_(ZERO)
	, new_volume_(ZERO)
{
}

void AudioVolume::SetVolume(double volume)
{
	new_volume_ = static_cast<uint32_t>(volume * ZERO);
}


double AudioVolume::ProcessVolume(const std::shared_ptr<AVFrame>& frame)
{
	assert(frame->format == AVSampleFormat::AV_SAMPLE_FMT_S32);
	int32_t* samples = reinterpret_cast<int32_t*>(frame->data[0]);
	int samples_count = frame->nb_samples * frame->channels;
	int64_t sum_volume = 0LL;
	for (int i = 0; i < samples_count; i++)
	{
		if (old_volume_ != new_volume_ && i > 0 && (samples[i] * samples[i - 1]) < 0) // sign of sample was changed, probably near zero, we can now adjust the volume
			old_volume_ = new_volume_;
		samples[i] = av_clipl_int32((((int64_t)samples[i] * old_volume_ / ZERO)));
		sum_volume += abs(samples[i]);
	}
	return static_cast<double>(sum_volume) / frame->nb_samples;
}

}}