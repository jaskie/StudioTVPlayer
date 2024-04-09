#pragma once
#include "../Core/AudioParameters.h"


namespace TVPlayR {
	namespace FFmpeg {

class AudioFifo final : private Common::NonCopyable, private Common::DebugTarget
{
public:
	/// <summary>
	/// Creates fifo for audio samples.
	/// </summary>
	/// <param name="seek_time">samples before the time will be ignored</param>
	/// <param name="capacity">fifo capacity in AV_TIME_BASE inits</param>
	AudioFifo(AVSampleFormat sample_format, int channel_count, int sample_rate, std::int64_t seek_time, std::int64_t capacity);
	bool Push(std::shared_ptr<AVFrame> frame);
	/// <summary>
	/// returns frame with requested nb_samples. If FIFO is smaller, rest will be filled with silence.
	/// </summary>
	/// <param name="nb_samples"></param>
	/// <returns></returns>
	std::shared_ptr<AVFrame> Pull(int nb_samples);
	std::shared_ptr<AVFrame> PullToTime(std::int64_t time);
	void DiscardSamples(int nb_samples);
	void Reset(std::int64_t seek_time);
	int SamplesCount() const;
	std::int64_t TimeMax() const;
	std::int64_t TimeMin() const;
	inline AVRational TimeBase() const { return time_base_; }
private:
	const std::unique_ptr<AVAudioFifo, void(*)(AVAudioFifo*)> audio_fifo_;
	const int sample_rate_;
	const AVRational time_base_;
	const AVSampleFormat sample_format_;
	const int channel_count_;
	AVChannelLayout channel_layout_;
	std::int64_t seek_time_;
	std::int64_t start_sample_ = 0LL;
	std::int64_t end_sample_ = 0LL;
};

}}