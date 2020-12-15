#include "../pch.h"
#include "AudioFifo.h"

namespace TVPlayR {
	namespace FFmpeg {

AudioFifo::AudioFifo(AVSampleFormat sample_fmt, int channels_count, int sample_rate, AVRational time_base, int64_t seek_time, int64_t fifo_duration)
	: fifo_(av_audio_fifo_alloc(sample_fmt, channels_count, static_cast<int>(fifo_duration * sample_rate / AV_TIME_BASE)), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); })
	, sample_rate_(sample_rate)
	, time_base_(time_base)
	, seek_time_(seek_time)
	, sample_fmt_(sample_fmt)
	, channels_count_(channels_count)
{
}

bool AudioFifo::TryPush(AVFramePtr frame)
{
	assert(frame->format == sample_fmt_);
	int fifo_space = av_audio_fifo_space(fifo_.get());
	int fifo_size = av_audio_fifo_size(fifo_.get());
	if (frame->nb_samples * 2 > fifo_space + fifo_size)
	{
		av_audio_fifo_realloc(fifo_.get(), frame->nb_samples * 2);
		fifo_space = av_audio_fifo_space(fifo_.get());
	}
	int64_t frame_start_time = TimeFromTs(frame->pts);
	int64_t frame_end_time = frame_start_time + av_rescale(frame->nb_samples, AV_TIME_BASE, sample_rate_);
	if (frame_end_time > seek_time_)
	{
		if (fifo_space < frame->nb_samples)
			return false;
		if (av_audio_fifo_write(fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
			THROW_EXCEPTION("Not all audio samples were written");
		end_sample_ += frame->nb_samples;
		if (frame_start_time <= seek_time_) // first frame
		{
			start_sample_ = av_rescale(frame_start_time, sample_rate_, AV_TIME_BASE);
			end_sample_ += start_sample_;
			int samples_to_discard = static_cast<int>((seek_time_ - frame_start_time) * sample_rate_ / AV_TIME_BASE);
			if (samples_to_discard > 0)
			{
				THROW_ON_FFMPEG_ERROR(av_audio_fifo_drain(fifo_.get(), samples_to_discard));
				start_sample_ += samples_to_discard;
			}
		}
	}
	return true;
}

AVFramePtr AudioFifo::Pull(int nb_samples)
{
	int samples_in_fifo = av_audio_fifo_size(fifo_.get());
	if (samples_in_fifo < nb_samples)
		nb_samples = samples_in_fifo;
	if (nb_samples <= 0)
		return __nullptr;
	auto frame(AllocFrame());
	frame->nb_samples = nb_samples;
	frame->format = sample_fmt_;
	frame->channels = channels_count_;
	frame->channel_layout = av_get_default_channel_layout(channels_count_);
	frame->sample_rate = sample_rate_;
	frame->pts = start_sample_ * time_base_.den / (sample_rate_ * time_base_.num);
	THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 32));
	int readed = av_audio_fifo_read(fifo_.get(), (void**)frame->data, frame->nb_samples);
	start_sample_ += readed;
	assert(readed == frame->nb_samples);
	return frame;
}

void AudioFifo::Reset(int64_t seek_time)
{
	av_audio_fifo_reset(fifo_.get());
	start_sample_ = 0LL;
	end_sample_ = 0LL;
	seek_time_ = seek_time;
}

int64_t AudioFifo::TimeFromTs(int64_t ts) const
{
	return av_rescale_q(ts, time_base_, av_get_time_base_q());
}

int AudioFifo::SamplesCount() const
{
	return av_audio_fifo_size(fifo_.get());
}


int64_t AudioFifo::TimeMin() const
{
	//TODO: sprawdziæ
	return av_rescale(start_sample_ * AV_TIME_BASE, time_base_.num, time_base_.den * sample_rate_);
}

int64_t AudioFifo::TimeMax() const
{
	return av_rescale(end_sample_ * AV_TIME_BASE, time_base_.num, time_base_.den * sample_rate_);
}


}}