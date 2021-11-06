#include "../pch.h"
#include "FFmpegUtils.h"
#include "VideoFilterBase.h"
#include "../Common/Rational.h"

namespace TVPlayR {
	namespace FFmpeg {

bool VideoFilterBase::Push(std::shared_ptr<AVFrame> frame) 
{ 
	if (frame->width != input_width_ ||
		frame->height != input_height_ ||
		frame->format != input_pixel_format_ ||
		frame->sample_aspect_ratio.num != input_sar_.num ||
		frame->sample_aspect_ratio.den != input_sar_.den
		)
		CreateFilter(frame->width, frame->height, static_cast<AVPixelFormat>(frame->format), frame->sample_aspect_ratio);
	return av_buffersrc_write_frame(source_ctx_, frame.get()) >= 0;
}

VideoFilterBase::VideoFilterBase(AVPixelFormat output_pix_fmt)
	: Common::DebugTarget(true, "VideoFilterBase")
	, output_pix_fmt_(output_pix_fmt)
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

AVRational VideoFilterBase::OutputFrameRate() const 
{ 
	assert(sink_ctx_);
	return av_buffersink_get_frame_rate(sink_ctx_);
}


void VideoFilterBase::Flush() { 
	if (source_ctx_)
		av_buffersrc_write_frame(source_ctx_, NULL);
	is_flushed_ = true;
}

void VideoFilterBase::SetFilter(const std::string& filter_str, const AVRational input_time_base)
{
	Reset();
	input_time_base_ = input_time_base;
	filter_ = filter_str;
	input_width_ = 0;
	input_height_ = 0;
	input_pixel_format_ = AV_PIX_FMT_NONE;
	input_sar_ = av_make_q(1, 1);
}

void VideoFilterBase::CreateFilter(int input_width, int input_height, AVPixelFormat input_pixel_format, const AVRational input_sar) 
{
	graph_ = std::unique_ptr<AVFilterGraph, void(*)(AVFilterGraph*)>(avfilter_graph_alloc(), [](AVFilterGraph* graph) { avfilter_graph_free(&graph); });
	AVFilterInOut* inputs = avfilter_inout_alloc();
	AVFilterInOut* outputs = avfilter_inout_alloc();
	try
	{
		const AVFilter* buffersrc = avfilter_get_by_name("buffer");
		const AVFilter* buffersink = avfilter_get_by_name("buffersink");
		char args[512];
		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			input_width, input_height, input_pixel_format,
			input_time_base_.num, input_time_base_.den,
			input_sar.num, input_sar.den);
		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&source_ctx_, buffersrc, "vin", args, NULL, graph_.get()));
		enum AVPixelFormat pix_fmts[] = { output_pix_fmt_, AV_PIX_FMT_NONE };
		THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&sink_ctx_, buffersink, "vout", NULL, NULL, graph_.get()));
		THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN));

		outputs->name = av_strdup("in");
		outputs->filter_ctx = source_ctx_;
		outputs->pad_idx = 0;
		outputs->next = NULL;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = sink_ctx_;
		inputs->pad_idx = 0;
		inputs->next = NULL;
		THROW_ON_FFMPEG_ERROR(avfilter_graph_parse(graph_.get(), filter_.c_str(), inputs, outputs, NULL));
		THROW_ON_FFMPEG_ERROR(avfilter_graph_config(graph_.get(), NULL));
		input_width_ = input_width;
		input_height_ = input_height;
		input_pixel_format_ = static_cast<AVPixelFormat>(input_pixel_format);
		input_sar_ = input_sar;
		DebugPrintLine(args);
		if (IsDebugOutput())
			DumpFilter(filter_, graph_.get());
	}
	catch (const std::exception& e)
	{
		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
		throw e;
	}
}


bool VideoFilterBase::IsInitialized() const 
{
	return !!graph_;
}

void VideoFilterBase::Reset() 
{ 
	source_ctx_ = NULL;
	sink_ctx_ = NULL;
	is_eof_ = false;
	is_flushed_ = false;
	graph_.reset();
	input_width_ = 0;
	input_height_ = 0;
	input_pixel_format_ = AV_PIX_FMT_NONE;
	input_sar_ = av_make_q(1, 1);
}

}}