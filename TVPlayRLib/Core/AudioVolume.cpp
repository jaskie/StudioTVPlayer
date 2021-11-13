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


std::vector<double> AudioVolume::ProcessVolume(const std::shared_ptr<AVFrame>& frame)
{
	assert(frame->format == AVSampleFormat::AV_SAMPLE_FMT_S32);
	int32_t* samples = reinterpret_cast<int32_t*>(frame->data[0]);
	int samples_count = frame->nb_samples * frame->channels;
	std::vector<std::int64_t> peak_volume_int64(frame->channels, 0LL);
	for (int i = 0; i < samples_count; i++)
	{
		if (volume_ != new_volume_ && i > 0 && (samples[i] * samples[i - 1]) < 0) // sign of sample was changed, probably near zero, we can now adjust the volume
			volume_ = new_volume_;
		samples[i] = av_clipl_int32((((std::int64_t)samples[i] * volume_ / ZERO)));
		if (abs(samples[i]) > peak_volume_int64[i % frame->channels])
			peak_volume_int64[i % frame->channels] = abs(samples[i]);
	}
	if (volume_ != new_volume_)
		volume_ = new_volume_; // in case when volume change failed within one frame period
	std::vector<double> peak_volume_double(frame->channels);
	std::transform(peak_volume_int64.begin(), peak_volume_int64.end(), peak_volume_double.begin(), [](std::int64_t volume) { return static_cast<double>(volume) / (std::numeric_limits<int32_t>().max)(); });
	return peak_volume_double;
}

}}