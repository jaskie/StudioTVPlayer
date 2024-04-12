#pragma once
#include "../Core/AudioParameters.h"
#include "FFmpegUtils.h"

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
	AudioFifo(AVRational input_time_base, AVSampleFormat sample_format, int channel_count, int sample_rate, std::int64_t seek_time, std::int64_t capacity);
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
	AVRational TimeBase() const { return output_time_base_; }
private:
	const unique_ptr<AVAudioFifo> audio_fifo_;
	const int sample_rate_;
	const AVRational input_time_base_;
	const AVRational output_time_base_;
	const AVSampleFormat sample_format_; // both output and input
	const int channel_count_; // both output and input
	const AVChannelLayout channel_layout_; // both output and input
	std::int64_t seek_time_;
	std::int64_t start_sample_ = 0LL;
	std::int64_t end_sample_ = 0LL;
};

}}