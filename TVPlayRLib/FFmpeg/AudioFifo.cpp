#include "../pch.h"
#include "AudioFifo.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

AudioFifo::AudioFifo(AVRational input_time_base, AVSampleFormat sample_format, int channel_count, int sample_rate, std::int64_t seek_time, std::int64_t capacity)
	: Common::DebugTarget(Common::DebugSeverity::debug, "AduioFifo")
	, input_time_base_(input_time_base)
	, sample_format_(sample_format)
	, channel_count_(channel_count)
	, sample_rate_(sample_rate)
	, audio_fifo_(av_audio_fifo_alloc(sample_format, channel_count, static_cast<int>(capacity * sample_rate / AV_TIME_BASE)), [](AVAudioFifo * fifo) {av_audio_fifo_free(fifo); })
	, output_time_base_(av_make_q(1, sample_rate_))
	, seek_time_(seek_time)
	, channel_layout_(GetChannelLayoutFromMask(channel_count))
{
}

bool AudioFifo::Push(std::shared_ptr<AVFrame> frame)
{
	assert(frame->format == sample_format_);
	assert(frame->sample_rate == sample_rate_);
	assert(frame->ch_layout.nb_channels == channel_count_);
    DebugPrintLine(Common::DebugSeverity::trace, "Pushed audio frame to fifo: " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE) + ", duration: " + std::to_string(av_rescale(frame->nb_samples, AV_TIME_BASE, frame->sample_rate / 1000)) + " ms");
	int fifo_space = av_audio_fifo_space(audio_fifo_.get());
	int fifo_size = av_audio_fifo_size(audio_fifo_.get());
	if (frame->nb_samples * 2 > fifo_space + fifo_size)
	{
		av_audio_fifo_realloc(audio_fifo_.get(), frame->nb_samples * 2);
		fifo_space = av_audio_fifo_space(audio_fifo_.get());
	}
	std::int64_t frame_start_time = FrameTime(frame);
	std::int64_t frame_end_time = frame_start_time + av_rescale(frame->nb_samples, AV_TIME_BASE, frame->sample_rate);
	if (frame_end_time > seek_time_)
	{
		if (fifo_space < frame->nb_samples)
			return false;
		if (av_audio_fifo_write(audio_fifo_.get(), (void**)frame->data, frame->nb_samples) != frame->nb_samples)
			THROW_EXCEPTION("AudioFifo: not all audio samples were written to fifo");
		end_sample_ += frame->nb_samples;
		if (frame_start_time <= seek_time_) // first frame
		{
			start_sample_ = av_rescale(frame_start_time, frame->sample_rate, AV_TIME_BASE);
			end_sample_ += start_sample_;
			int samples_to_discard = static_cast<int>((seek_time_ - frame_start_time) * frame->sample_rate / AV_TIME_BASE);
			if (samples_to_discard > 0)
			{
				THROW_ON_FFMPEG_ERROR(av_audio_fifo_drain(audio_fifo_.get(), samples_to_discard));
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
	int samples_in_fifo = av_audio_fifo_size(audio_fifo_.get());
	if (nb_samples < 0)
		return nullptr;
	auto frame = AllocFrame();
	frame->nb_samples = nb_samples;
	frame->format = sample_format_;
	frame->ch_layout = channel_layout_;
	frame->sample_rate = sample_rate_;
	frame->time_base = output_time_base_;
	frame->pts = av_rescale(start_sample_, output_time_base_.den, static_cast<std::int64_t>(sample_rate_) * output_time_base_.num);
	int samples_from_fifo = min(samples_in_fifo, nb_samples);
	if (nb_samples > 0)
	{
		THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 0));
		int readed = av_audio_fifo_read(audio_fifo_.get(), (void**)frame->data, samples_from_fifo);
		if (readed >= 0)
			start_sample_ += readed;
		assert(readed == samples_from_fifo);
	}
	if (samples_from_fifo < nb_samples)
	{
		av_samples_set_silence(frame->data, samples_from_fifo, nb_samples - samples_from_fifo, channel_layout_.nb_channels, sample_format_);
		DebugPrintLine(Common::DebugSeverity::debug, "Filled audio with silence at time: " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE) + ", duration: " + std::to_string(av_rescale(nb_samples - samples_from_fifo, AV_TIME_BASE, frame->sample_rate) / 1000) + " ms");
	}
	else
		DebugPrintLine(Common::DebugSeverity::trace, "Pulled audio frame from fifo at time: " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE) + ", duration: " + std::to_string(av_rescale(frame->nb_samples, AV_TIME_BASE, frame->sample_rate) / 1000) + " ms");
	return frame;
}

std::shared_ptr<AVFrame> AudioFifo::PullToTime(std::int64_t time)
{
	std::int64_t to_sample = av_rescale(time, sample_rate_, AV_TIME_BASE);
	assert(to_sample >= start_sample_);
	return Pull(static_cast<int>(to_sample - start_sample_));
}

void AudioFifo::DiscardSamples(int nb_samples)
{
	int samples_in_fifo = av_audio_fifo_size(audio_fifo_.get());
	if (samples_in_fifo < nb_samples)
		nb_samples = samples_in_fifo;
	if (nb_samples <= 0)
		return;
	DebugPrintLine(Common::DebugSeverity::debug,"Audio samples discarded: " + std::to_string(nb_samples));
	if (av_audio_fifo_drain(audio_fifo_.get(), nb_samples) == 0)
		start_sample_ += nb_samples;
}

void AudioFifo::Reset(std::int64_t seek_time)
{
	av_audio_fifo_reset(audio_fifo_.get());
	start_sample_ = 0LL;
	end_sample_ = 0LL;
	seek_time_ = seek_time;
}

int AudioFifo::SamplesCount() const
{
	return av_audio_fifo_size(audio_fifo_.get());
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