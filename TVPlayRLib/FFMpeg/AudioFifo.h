#pragma once
#include "Utils.h"


namespace TVPlayR {
	namespace FFmpeg {

class AudioFifo
{
public:
	AudioFifo(AVSampleFormat sample_fmt, int channels_count, int sample_rate, AVRational time_base, int64_t seek_time, int64_t fifo_duration);
	bool TryPush(std::shared_ptr<AVFrame> frame);
	std::shared_ptr<AVFrame> Pull(int nb_samples);
	void DiscardSamples(int nb_samples);
	void Reset(int64_t seek_time);
	int SamplesCount() const;
	int64_t TimeMax() const;
	int64_t TimeMin() const;
	inline AVRational TimeBase() const { return time_base_; }
private:
	AVAudioFifoPtr aduio_fifo_;
	const AVRational time_base_;
	const int sample_rate_;
	const AVSampleFormat sample_fmt_;
	const int channels_count_;
	int64_t seek_time_;
	int64_t start_sample_ = 0LL;
	int64_t end_sample_ = 0LL;
};

}}