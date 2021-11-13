#pragma once
#include "../Common/Debug.h"
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace FFmpeg {

class AudioFifo final : private Common::NonCopyable, private Common::DebugTarget
{
public:
	AudioFifo(AVSampleFormat sample_fmt, int channels_count, int sample_rate, AVRational time_base, std::int64_t seek_time, std::int64_t fifo_duration);
	bool TryPush(std::shared_ptr<AVFrame> frame);
	/// <summary>
	/// returns frame with requested nb_samples. If FIFO is smaller, rest will be filled with silence.
	/// </summary>
	/// <param name="nb_samples"></param>
	/// <returns></returns>
	std::shared_ptr<AVFrame> Pull(int nb_samples);
	void DiscardSamples(int nb_samples);
	void Reset(std::int64_t seek_time);
	int SamplesCount() const;
	std::int64_t TimeMax() const;
	std::int64_t TimeMin() const;
	inline AVRational TimeBase() const { return time_base_; }
private:
	const std::unique_ptr<AVAudioFifo, void(*)(AVAudioFifo*)> aduio_fifo_;
	const AVRational time_base_;
	const int sample_rate_;
	const AVSampleFormat sample_fmt_;
	const int channels_count_;
	std::int64_t seek_time_;
	std::int64_t start_sample_ = 0LL;
	std::int64_t end_sample_ = 0LL;
};

}}