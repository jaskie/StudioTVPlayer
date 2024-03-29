#include "../pch.h"
#include "AudioFifo.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

AudioFifo::AudioFifo(AVSampleFormat sample_fmt, int channels_count, int sample_rate, AVRational time_base, std::int64_t seek_time, std::int64_t fifo_duration)
	: Common::DebugTarget(Common::DebugSeverity::info, "AduioFifo")
	, aduio_fifo_(av_audio_fifo_alloc(sample_fmt, channels_count, static_cast<int>(fifo_duration * sample_rate / AV_TIME_BASE)), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); })
	, sample_rate_(sample_rate)
	, time_base_(time_base)
	, seek_time_(seek_time)
	, sample_fmt_(sample_fmt)
{
	av_channel_layout_default(&channel_layout_, channels_count);
}

bool AudioFifo::TryPush(std::shared_ptr<AVFrame> frame)
{
	assert(frame->format == sample_fmt_);
    DebugPrintLine(Common::DebugSeverity::trace, "Pushed audio frame to fifo: " + std::to_string(static_cast<float>(PtsToTime(frame->pts, time_base_)) / AV_TIME_BASE) + ", duration: " + std::to_string(PtsToTime(frame->duration, time_base_) / 1000) + " ms");
	int fifo_space = av_audio_fifo_space(aduio_fifo_.get());
	int fifo_size = av_audio_fifo_size(aduio_fifo_.get());
	if (frame->nb_samples * 2 > fifo_space + fifo_size)
	{
		av_audio_fifo_realloc(aduio_fifo_.get(), frame->nb_samples * 2);
		fifo_space = av_audio_fifo_space(aduio_fifo_.get());
	}
	std::int64_t frame_start_time = PtsToTime(frame->pts, time_base_);
	std::int64_t frame_end_time = frame_start_time + av_rescale(frame->nb_samples, AV_TIME_BASE, sample_rate_);
	if (frame_end_time > seek_time_)
	{
		if (fifo_space < frame->nb_samples)
			return false;
		if (av_audio_fifo_write(aduio_fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
			THROW_EXCEPTION("AudioFifo: not all audio samples were written to fifo");
		end_sample_ += frame->nb_samples;
		if (frame_start_time <= seek_time_) // first frame
		{
			start_sample_ = av_rescale(frame_start_time, sample_rate_, AV_TIME_BASE);
			end_sample_ += start_sample_;
			int samples_to_discard = static_cast<int>((seek_time_ - frame_start_time) * sample_rate_ / AV_TIME_BASE);
			if (samples_to_discard > 0)
			{
				THROW_ON_FFMPEG_ERROR(av_audio_fifo_drain(aduio_fifo_.get(), samples_to_discard));
				start_sample_ += samples_to_discard;
			}
		}
	}
	else
		DebugPrintLine(Common::DebugSeverity::debug, "Frame ignored");
	return true;
}

std::shared_ptr<AVFrame> AudioFifo::Pull(int nb_samples)
{
	int samples_in_fifo = av_audio_fifo_size(aduio_fifo_.get());
	if (nb_samples < 0)
		return nullptr;
	auto frame(AllocFrame());
	frame->nb_samples = nb_samples;
	frame->format = sample_fmt_;
	frame->ch_layout = channel_layout_;
	frame->sample_rate = sample_rate_;
	frame->pts = av_rescale(start_sample_, time_base_.den, static_cast<std::int64_t>(sample_rate_) * time_base_.num);
	int samples_from_fifo = min(samples_in_fifo, nb_samples);
	if (nb_samples > 0)
	{
		THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 0));
		int readed = av_audio_fifo_read(aduio_fifo_.get(), (void**)frame->data, samples_from_fifo);
		if (readed >= 0)
			start_sample_ += readed;
		assert(readed == samples_from_fifo);
	}
	if (samples_from_fifo < nb_samples)
	{
		av_samples_set_silence(frame->data, samples_from_fifo, nb_samples - samples_from_fifo, channel_layout_.nb_channels, sample_fmt_);
		DebugPrintLine(Common::DebugSeverity::debug, "Filled audio with silence at time: " + std::to_string(static_cast<float>(PtsToTime(frame->pts, time_base_)) / AV_TIME_BASE) + ", duration: " + std::to_string(av_rescale(nb_samples - samples_from_fifo, AV_TIME_BASE, frame->sample_rate) / 1000) + " ms");
	}
	else
		DebugPrintLine(Common::DebugSeverity::trace, "Pulled audio frame from fifo at time: " + std::to_string(static_cast<float>(PtsToTime(frame->pts, time_base_)) / AV_TIME_BASE) + ", duration: " + std::to_string(av_rescale(frame->nb_samples, AV_TIME_BASE, frame->sample_rate) / 1000) + " ms");
	return frame;
}

void AudioFifo::DiscardSamples(int nb_samples)
{
	int samples_in_fifo = av_audio_fifo_size(aduio_fifo_.get());
	if (samples_in_fifo < nb_samples)
		nb_samples = samples_in_fifo;
	if (nb_samples <= 0)
		return;
	DebugPrintLine(Common::DebugSeverity::debug,"Audio samples discarded: " + std::to_string(nb_samples));
	if (av_audio_fifo_drain(aduio_fifo_.get(), nb_samples) == 0)
		start_sample_ += nb_samples;
}

void AudioFifo::Reset(std::int64_t seek_time)
{
	av_audio_fifo_reset(aduio_fifo_.get());
	start_sample_ = 0LL;
	end_sample_ = 0LL;
	seek_time_ = seek_time;
}

int AudioFifo::SamplesCount() const
{
	return av_audio_fifo_size(aduio_fifo_.get());
}

std::int64_t AudioFifo::TimeMin() const
{
	return av_rescale(start_sample_, AV_TIME_BASE , sample_rate_);
}


std::int64_t AudioFifo::TimeMax() const
{
	return av_rescale(end_sample_, AV_TIME_BASE, sample_rate_);
}


}}