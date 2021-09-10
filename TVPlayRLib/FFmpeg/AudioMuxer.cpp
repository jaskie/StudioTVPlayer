#include "../pch.h"
#include "AudioMuxer.h"
#include "Decoder.h"
#include "AudioFifo.h"
#include "FFmpegUtils.h"


namespace TVPlayR {
	namespace FFmpeg {

AudioMuxer::AudioMuxer(const std::vector<std::unique_ptr<Decoder>>& decoders, int64_t output_channel_layout, const AVSampleFormat sample_format, const int sample_rate, const int nb_channels)
	: FilterBase::FilterBase()
	, decoders_(decoders)
	, input_time_base_(decoders.empty() ? av_make_q(1, sample_rate) : decoders[0]->TimeBase())
	, output_sample_rate_(sample_rate)
	, nb_channels_(nb_channels)
	, output_channel_layout_(output_channel_layout)
	, audio_sample_format_(sample_format)
{ 
	filter_str_ = GetAudioMuxerString(sample_rate);
	Reset();
}

std::string AudioMuxer::GetAudioMuxerString(const int sample_rate)
{
	std::ostringstream filter;
	int total_nb_channels = std::accumulate(decoders_.begin(), decoders_.end(), 0, [](int sum, const std::unique_ptr<Decoder>& curr) { return sum + curr->AudioChannelsCount(); });
	if (decoders_.size() > 1)
	{
		for (int i = 0; i < decoders_.size(); i++)
			filter << "[a" << i << "]";
		filter << "amerge=inputs=" << decoders_.size() << ",";
		if (total_nb_channels > 2)
			filter << "channelmap=0|1:stereo,";
		if (total_nb_channels == 1)
			filter << "channelmap=0|0:stereo,";
	}
	else
	{
		if (total_nb_channels > 2)
			filter << "[a0]channelmap=0|1:stereo,";
		if (total_nb_channels == 1)
			filter << "[a0]channelmap=0|0:stereo,";
		if (total_nb_channels == 2)
			filter << "[a0]";
	}
	filter << "aresample=out_sample_fmt=" << av_get_sample_fmt_name(audio_sample_format_) << ":out_sample_rate=" << sample_rate;
	return filter.str();
}

int AudioMuxer::OutputSampleRate()
{
	return av_buffersink_get_sample_rate(sink_ctx_);
}

int AudioMuxer::OutputChannelsCount()
{
	return av_buffersink_get_channels(sink_ctx_);
}

AVRational AudioMuxer::OutputTimeBase() const
{
	return av_buffersink_get_time_base(sink_ctx_);
}

uint64_t AudioMuxer::OutputChannelLayout()
{
	return av_buffersink_get_channel_layout(sink_ctx_);
}

AVSampleFormat AudioMuxer::OutputSampleFormat()
{
	return audio_sample_format_;
}

void AudioMuxer::Push(int stream_index, std::shared_ptr<AVFrame> frame)
{
	auto dest = std::find_if(std::begin(source_ctx_), std::end(source_ctx_), [&stream_index](const std::pair<int, AVFilterContext*>& ctx) {return ctx.first == stream_index; });
	if (dest == std::end(source_ctx_))
	THROW_EXCEPTION("AudioMuxer: stream not found");
	DebugPrintLine(("Pushed to muxer:   " + std::to_string(PtsToTime(frame->pts, input_time_base_) / 1000)));
	int ret = av_buffersrc_write_frame(dest->second, frame.get());
	switch (ret)
	{
	case 0:
		break;
	case AVERROR(EINVAL):
		Initialize();
		Push(stream_index, frame);
		break;
	default:
		THROW_ON_FFMPEG_ERROR(ret);
	}		
}

std::shared_ptr<AVFrame> AudioMuxer::Pull()
{
	auto frame = AllocFrame();
	int ret = av_buffersink_get_frame(sink_ctx_, frame.get());
	switch (ret)
	{
		case 0: 
			DebugPrintLine(("Pulled from muxer: " + std::to_string(PtsToTime(frame->pts, av_buffersink_get_time_base(sink_ctx_)) / 1000)));
			return frame;
		case AVERROR(EAGAIN):
			break;
		case AVERROR_EOF:
			is_eof_ = true;
			break;
		default:
			THROW_ON_FFMPEG_ERROR(ret);
			break;
	}
	if (ret < 0)
		return nullptr;
	return frame;
}

void AudioMuxer::Flush()
{
	if (is_flushed_)
		return;
	is_flushed_ = true;
	for (const auto& ctx : source_ctx_)
		av_buffersrc_write_frame(ctx.second, NULL);
}

void AudioMuxer::Reset()
{
	Initialize();
}


void AudioMuxer::Initialize()
{
	if (std::find_if(decoders_.begin(), decoders_.end(), [](const std::unique_ptr<Decoder>& decoder) -> bool { return decoder->MediaType() != AVMEDIA_TYPE_AUDIO; }) != decoders_.end())
		THROW_EXCEPTION("AudioMuxer::CreateFilterChain() got non-audio stream")

	source_ctx_.clear();
	is_eof_ = false;
	is_flushed_ = false;
	graph_.reset(avfilter_graph_alloc());

	AVSampleFormat out_sample_fmts[] = { audio_sample_format_, AV_SAMPLE_FMT_NONE };
	int64_t out_channel_layouts[] = { output_channel_layout_ , -1 };
	int out_sample_rates[] = { output_sample_rate_, -1 };

	AVFilterInOut * inputs = avfilter_inout_alloc();
	AVFilterInOut * outputs = avfilter_inout_alloc();

	try
	{
		const AVFilter *buffersrc = avfilter_get_by_name("abuffer");
		const AVFilter *buffersink = avfilter_get_by_name("abuffersink");

		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&sink_ctx_, buffersink, "aout", NULL, NULL, graph_.get()));
		THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "sample_fmts", out_sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN));
		THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN));
		THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN));

		char args[512];
		AVFilterInOut * current_output = outputs;
		for (int i = 0; i < decoders_.size(); i++)
		{
			auto channel_layout = decoders_[i]->AudioChannelLayout();
			if (!channel_layout)
				channel_layout = av_get_default_channel_layout(decoders_[i]->AudioChannelsCount());
			snprintf(args, sizeof(args),
				"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx",
				decoders_[i]->TimeBase().num, decoders_[i]->TimeBase().den, decoders_[i]->AudioSampleRate(),
				av_get_sample_fmt_name(decoders_[i]->AudioSampleFormat()), channel_layout);
			auto new_source = std::pair<int, AVFilterContext*>(decoders_[i]->StreamIndex(), NULL);
			THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&new_source.second, buffersrc, "ain", args, NULL, graph_.get()));

			snprintf(args, sizeof(args), "a%d", i);
			current_output->name = av_strdup(args);
			current_output->filter_ctx = new_source.second;
			current_output->pad_idx = 0;
			if (i == decoders_.size() - 1)
				current_output->next = NULL;
			else
			{
				current_output->next = avfilter_inout_alloc();
				current_output = current_output->next;
			}
			//THROW_ON_FFMPEG_ERROR(avfilter_link(new_source.second, 0, sink_ctx_, i));
			source_ctx_.push_back(new_source);
		}

		inputs->name = av_strdup("out");
		inputs->filter_ctx = sink_ctx_;
		inputs->pad_idx = 0;
		inputs->next = NULL;
		if (avfilter_graph_parse_ptr(graph_.get(), filter_str_.c_str(), &inputs, &outputs, NULL) < 0)
			THROW_EXCEPTION("avfilter_graph_parse_ptr failed")
		if (avfilter_graph_config(graph_.get(), NULL) < 0)
			THROW_EXCEPTION("avfilter_graph_config failed")
		if (IsDebugOutput())
			dump_filter(filter_str_, graph_.get());
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
	}
}

}}