#include "../pch.h"
#include "../Core/AudioParameters.h"
#include "AudioMuxer.h"
#include "Decoder.h"
#include "AudioFifo.h"
#include "FFmpegUtils.h"


namespace TVPlayR {
	namespace FFmpeg {

AudioMuxer::AudioMuxer(const std::vector<std::unique_ptr<Decoder>>& decoders, std::int64_t output_channel_layout, const Core::AudioParameters output_audio_parameters)
	: Common::DebugTarget(Common::DebugSeverity::info, "Audio muxer")
	, FilterBase::FilterBase()
	, input_time_base_(decoders.empty() ? av_make_q(1, output_audio_parameters.SampleRate) : decoders[0]->TimeBase())
	, output_audio_parameters_(output_audio_parameters)
	, output_channel_layout_(output_channel_layout)
	, filter_str_(GetAudioMuxerString(decoders))
	, sink_ctx_(NULL, [](AVFilterContext* filter) { avfilter_free(filter); })
	, inputs_(NULL, [](AVFilterInOut* io) { avfilter_inout_free(&io); })
	, outputs_(NULL, [](AVFilterInOut* io) { avfilter_inout_free(&io); })
{
	InitializeGraph(decoders);
}

std::string AudioMuxer::GetAudioMuxerString(const std::vector<std::unique_ptr<Decoder>>& decoders)
{
	std::ostringstream filter;
	int total_nb_channels = std::accumulate(decoders.begin(), decoders.end(), 0, [](int sum, const std::unique_ptr<Decoder>& curr) { return sum + curr->AudioChannelsCount(); });
	if (decoders.size() > 1)
	{
		for (int i = 0; i < decoders.size(); i++)
			filter << "[a" << i << "]";
		filter << "amerge=inputs=" << decoders.size() << ",";
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
	filter << "aresample=out_sample_fmt=" << av_get_sample_fmt_name(output_audio_parameters_.SampleFormat) << ":out_sample_rate=" << output_audio_parameters_.SampleRate;
	return filter.str();
}

int AudioMuxer::OutputSampleRate()
{
	return av_buffersink_get_sample_rate(sink_ctx_.get());
}

int AudioMuxer::OutputChannelsCount()
{
	return av_buffersink_get_channels(sink_ctx_.get());
}

AVRational AudioMuxer::OutputTimeBase() const
{
	return av_buffersink_get_time_base(sink_ctx_.get());
}

AVSampleFormat AudioMuxer::OutputSampleFormat() const
{
	return output_audio_parameters_.SampleFormat;
}

void AudioMuxer::Push(int stream_index, std::shared_ptr<AVFrame> frame)
{
	auto dest = std::find_if(std::begin(source_ctx_), std::end(source_ctx_), [&stream_index](const std::pair<int, unique_ptr<AVFilterContext>>& ctx) {return ctx.first == stream_index; });
	if (dest == std::end(source_ctx_))
		THROW_EXCEPTION("AudioMuxer: stream not found");
	DebugPrintLine(Common::DebugSeverity::trace, "Pushed to muxer:   " + std::to_string(PtsToTime(frame->pts, input_time_base_) / 1000));
	int ret = av_buffersrc_write_frame(dest->second.get(), frame.get());
	switch (ret)
	{
	case 0:
		break;
	case AVERROR(EINVAL):
		Push(stream_index, frame);
		break;
	default:
		THROW_ON_FFMPEG_ERROR(ret);
	}		
}

std::shared_ptr<AVFrame> AudioMuxer::Pull()
{
	auto frame = AllocFrame();
	int ret = av_buffersink_get_frame(sink_ctx_.get(), frame.get());
	switch (ret)
	{
		case 0: 
			DebugPrintLine(Common::DebugSeverity::trace, "Pulled from muxer: " + std::to_string(PtsToTime(frame->pts, av_buffersink_get_time_base(sink_ctx_.get())) / 1000));
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
		av_buffersrc_write_frame(ctx.second.get(), NULL);
}

void AudioMuxer::InitializeGraph(const std::vector<std::unique_ptr<Decoder>>& decoders)
{
	if (std::find_if(decoders.begin(), decoders.end(), [](const std::unique_ptr<Decoder>& decoder) -> bool { return decoder->MediaType() != AVMEDIA_TYPE_AUDIO; }) != decoders.end())
		THROW_EXCEPTION("AudioMuxer::InitializeGraph() got non-audio stream")

		source_ctx_.clear();
	is_eof_ = false;
	is_flushed_ = false;
	graph_.reset(avfilter_graph_alloc());

	AVSampleFormat out_sample_fmts[] = { output_audio_parameters_.SampleFormat, AV_SAMPLE_FMT_NONE };
	std::int64_t out_channel_layouts[] = { output_channel_layout_ , -1 };
	int out_sample_rates[] = { output_audio_parameters_.SampleRate, -1 };
	AVFilterInOut* inputs = avfilter_inout_alloc();
	inputs_.reset(inputs);
	AVFilterInOut* outputs = avfilter_inout_alloc();
	outputs_.reset(outputs);
	const AVFilter* buffersrc = avfilter_get_by_name("abuffer");
	const AVFilter* buffersink = avfilter_get_by_name("abuffersink");
	AVFilterContext* weak_sink_ctx = NULL;
	THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&weak_sink_ctx, buffersink, "aout", NULL, NULL, graph_.get()));
	sink_ctx_.reset(weak_sink_ctx);
	THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(weak_sink_ctx, "sample_fmts", out_sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN));
	THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(weak_sink_ctx, "channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN));
	THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(weak_sink_ctx, "sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN));

	char args[512];
	AVFilterInOut* current_output = outputs;
	for (int i = 0; i < decoders.size(); i++)
	{
		const std::unique_ptr<Decoder>& decoder = decoders[i];
		AVChannelLayout* ch_layout = decoder->AudioChannelLayout();
		if (!ch_layout)
			THROW_EXCEPTION("AudioMuxer: decoder's AudioChannelLayout empty");
		int ret = snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=",
			decoder->TimeBase().num, decoder->TimeBase().den, decoder->AudioSampleRate(),
			av_get_sample_fmt_name(decoder->AudioSampleFormat()));
		av_channel_layout_describe(ch_layout, args + ret, sizeof(args) - ret);
		AVFilterContext* weak_source = NULL;
		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&weak_source, buffersrc, "ain", args, NULL, graph_.get()));
		snprintf(args, sizeof(args), "a%d", i);
		current_output->name = av_strdup(args);
		current_output->filter_ctx = weak_source;
		current_output->pad_idx = 0;
		source_ctx_.emplace_back(std::pair<int, unique_ptr<AVFilterContext>>(
			decoder->StreamIndex(),
			unique_ptr<AVFilterContext>(weak_source, [](AVFilterContext* filter) { avfilter_free(filter); })));
		if (i == decoders.size() - 1)
			current_output->next = NULL;
		else
		{
			current_output->next = avfilter_inout_alloc();
			current_output = current_output->next;
		}
	}

	inputs->name = av_strdup("out");
	inputs->filter_ctx = weak_sink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;
	if (avfilter_graph_parse_ptr(graph_.get(), filter_str_.c_str(), &inputs, &outputs, NULL) < 0)
		THROW_EXCEPTION("AudioMuxer: avfilter_graph_parse_ptr failed")
		if (avfilter_graph_config(graph_.get(), NULL) < 0)
			THROW_EXCEPTION("AudioMuxer: avfilter_graph_config failed")
			if (DebugSeverity() <= Common::DebugSeverity::debug)
				DumpFilter(filter_str_, graph_.get());
}

}}