#include "../pch.h"
#include "FFmpegUtils.h"
#include "VideoFilterBase.h"
#include "../Common/Rational.h"

namespace TVPlayR {
	namespace FFmpeg {

bool VideoFilterBase::Push(std::shared_ptr<AVFrame> frame) 
{ 
	return av_buffersrc_write_frame(source_ctx_, frame.get()) >= 0;
}

VideoFilterBase::VideoFilterBase(AVPixelFormat output_pix_fmt)
	: output_pix_fmt_(output_pix_fmt)
{ }

std::shared_ptr<AVFrame> VideoFilterBase::Pull() {
	if (!sink_ctx_)
		return nullptr;
	auto frame = AllocFrame();
	auto ret = av_buffersink_get_frame(sink_ctx_, frame.get());
	switch (ret)
	{
	case AVERROR_EOF:
		is_eof_ = true;
		return nullptr;
	case AVERROR(EAGAIN):
		break;
	case AVERROR(EINVAL):
		return nullptr;
	}
	if (FF(ret))
	{
		//if (frame->best_effort_timestamp == AV_NOPTS_VALUE)
		//	frame->best_effort_timestamp = frame->pts;
		//frame->pts = av_rescale_q(frame->best_effort_timestamp, input_time_base_, av_buffersink_get_time_base(sink_ctx_));
		DebugPrintLine(("Pulled from VideoFilterBase: " + std::to_string(PtsToTime(frame->pts, av_buffersink_get_time_base(sink_ctx_)) / 1000) + "\n"));
		return frame;
	}
	return nullptr;
}

int VideoFilterBase::OutputWidth() { 
	assert(sink_ctx_);
	return av_buffersink_get_w(sink_ctx_);
}

int VideoFilterBase::OutputHeight() { 
	assert(sink_ctx_);
	return av_buffersink_get_h(sink_ctx_);
}

AVRational VideoFilterBase::OutputSampleAspectRatio() 
{ 
	assert(sink_ctx_);
	return av_buffersink_get_sample_aspect_ratio(sink_ctx_);
}

AVPixelFormat VideoFilterBase::OutputPixelFormat() 
{ 
	assert(sink_ctx_);
	return static_cast<AVPixelFormat>(av_buffersink_get_format(sink_ctx_));
}

AVRational VideoFilterBase::OutputTimeBase() const 
{ 
	assert(sink_ctx_);
	return av_buffersink_get_time_base(sink_ctx_);
}

void VideoFilterBase::Flush() { 
	if (source_ctx_)
		av_buffersrc_write_frame(source_ctx_, NULL);
	is_flushed_ = true;
}

void VideoFilterBase::CreateFilterChain(std::shared_ptr<AVFrame> frame, const Common::Rational<int> input_time_base, const std::string& filter_str)
{
	source_ctx_ = NULL;
	sink_ctx_ = NULL;
	is_eof_ = false;
	is_flushed_ = false;

	graph_.reset(avfilter_graph_alloc());
	AVFilterInOut* inputs = avfilter_inout_alloc();
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVBufferSinkParams* buffersink_params = av_buffersink_params_alloc();
	try
	{
		const AVFilter* buffersrc = avfilter_get_by_name("buffer");
		const AVFilter* buffersink = avfilter_get_by_name("buffersink");
		char args[512];
		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			frame->width, frame->height, frame->format,
			input_time_base.Numerator(), input_time_base.Denominator(),
			frame->sample_aspect_ratio.num, frame->sample_aspect_ratio.den);
		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&source_ctx_, buffersrc, "vin", args, NULL, graph_.get()));
		enum AVPixelFormat pix_fmts[] = { output_pix_fmt_, AV_PIX_FMT_NONE };
		buffersink_params->pixel_fmts = pix_fmts;
		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&sink_ctx_, buffersink, "vout", NULL, buffersink_params, graph_.get()));
		THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN));

		outputs->name = av_strdup("in");
		outputs->filter_ctx = source_ctx_;
		outputs->pad_idx = 0;
		outputs->next = NULL;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = sink_ctx_;
		inputs->pad_idx = 0;
		inputs->next = NULL;
		THROW_ON_FFMPEG_ERROR(avfilter_graph_parse(graph_.get(), filter_str.c_str(), inputs, outputs, NULL));
		THROW_ON_FFMPEG_ERROR(avfilter_graph_config(graph_.get(), NULL));
		DebugPrintLine(args);
		if (IsDebugOutput())
			dump_filter(filter_str, graph_.get());
	}
	catch (const std::exception& e)
	{
		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
		throw e;
	}
	av_free(buffersink_params);
}

bool VideoFilterBase::IsInitialized() const { return !!graph_; }

void VideoFilterBase::Reset() 
{ 
	source_ctx_ = NULL;
	sink_ctx_ = NULL;
	is_eof_ = false;
	is_flushed_ = false;
	graph_.reset();
}

}}