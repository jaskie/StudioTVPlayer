#include "../pch.h"
#include "VideoFilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {

VideoFilterBase::VideoFilterBase(AVPixelFormat output_pix_fmt)
	: Common::DebugTarget(Common::DebugSeverity::info, "VideoFilterBase")
	, output_pix_fmt_(output_pix_fmt)
	, source_ctx_(NULL, [](AVFilterContext* filter) { avfilter_free(filter); })
	, sink_ctx_(NULL, [](AVFilterContext* filter) { avfilter_free(filter); })
	, inputs_(NULL, [](AVFilterInOut* io) { avfilter_inout_free(&io); })
	, outputs_(NULL, [](AVFilterInOut* io) { avfilter_inout_free(&io); })
{ }


void VideoFilterBase::Push(const std::shared_ptr<AVFrame> &frame)
{ 
	std::lock_guard<std::mutex> lock(frame_queue_mutex_);
	frame_buffer_.emplace_back(frame);
	assert(frame_buffer_.size() < 100);
}

std::shared_ptr<AVFrame> VideoFilterBase::Pull()
{
	if (!sink_ctx_ && !PushFrameFromBuffer())
	{
		return nullptr;
	}
	auto frame = AllocFrame();
	while (true)
	{
		int ret = av_buffersink_get_frame(sink_ctx_.get(), frame.get());
		switch (ret)
		{
		case AVERROR_EOF:
			is_eof_ = true;
			return nullptr;
		case AVERROR(EAGAIN):
			if (!PushFrameFromBuffer())
				return nullptr;
			continue;
		case AVERROR(EINVAL):
			return nullptr;
		}
		if (FF_SUCCESS(ret))
		{
			//if (frame->best_effort_timestamp == AV_NOPTS_VALUE)
			//	frame->best_effort_timestamp = frame->pts;
			frame->time_base = OutputTimeBase();
			frame->pts = av_rescale_q(frame->pts, input_time_base_, frame->time_base);
			DebugPrintLine(Common::DebugSeverity::trace, "Pull: " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE));
			return frame;
		}
	}
	return nullptr;
}

int VideoFilterBase::OutputWidth()
{
	assert(sink_ctx_);
	return av_buffersink_get_w(sink_ctx_.get());
}

int VideoFilterBase::OutputHeight()
{ 
	assert(sink_ctx_);
	return av_buffersink_get_h(sink_ctx_.get());
}

AVRational VideoFilterBase::OutputSampleAspectRatio()
{ 
	assert(sink_ctx_);
	return av_buffersink_get_sample_aspect_ratio(sink_ctx_.get());
}

AVPixelFormat VideoFilterBase::OutputPixelFormat()
{ 
	return output_pix_fmt_;
}

AVRational VideoFilterBase::OutputTimeBase() const
{ 
	assert(sink_ctx_);
	return av_buffersink_get_time_base(sink_ctx_.get());
}

AVRational VideoFilterBase::OutputFrameRate() const
{ 
	assert(sink_ctx_);
	return av_buffersink_get_frame_rate(sink_ctx_.get());
}

void VideoFilterBase::Flush()
{
	if (source_ctx_)
		av_buffersrc_write_frame(source_ctx_.get(), NULL);
	is_flushed_ = true;
}

void VideoFilterBase::SetFilter(const std::string &filter_str)
{
	Clear();
	filter_ = filter_str;
	input_width_ = 0;
	input_height_ = 0;
	input_pixel_format_ = AV_PIX_FMT_NONE;
	input_sar_ = av_make_q(1, 1);
}

void VideoFilterBase::Initialize(const std::shared_ptr<AVFrame>& frame)
{
	graph_.reset(avfilter_graph_alloc());
	outputs_.reset(avfilter_inout_alloc());
	inputs_.reset(avfilter_inout_alloc());
	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	std::stringstream args;
	args << "video_size=" << frame->width << "x" << frame->height
		<< ":pix_fmt=" << static_cast<AVPixelFormat>(frame->format)
		<< ":time_base=" << frame->time_base.num << "/" << frame->time_base.den
		<< ":pixel_aspect=" << frame->sample_aspect_ratio.num << "/" << frame->sample_aspect_ratio.den;
	AVFilterContext* weak_source = NULL;
	THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&weak_source, buffersrc, "vin", args.str().c_str(), NULL, graph_.get()));
	source_ctx_.reset(weak_source);
	enum AVPixelFormat pix_fmts[] = { output_pix_fmt_, AV_PIX_FMT_NONE };
	AVFilterContext* weak_sink = NULL;
	THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&weak_sink, buffersink, "vout", NULL, NULL, graph_.get()));
	THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(weak_sink, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN));
	sink_ctx_.reset(weak_sink);

	inputs_->name = av_strdup("in");
	inputs_->filter_ctx = weak_source;
	inputs_->pad_idx = 0;
	inputs_->next = NULL;

	outputs_->name = av_strdup("out");
	outputs_->filter_ctx = weak_sink;
	outputs_->pad_idx = 0;
	outputs_->next = NULL;
	THROW_ON_FFMPEG_ERROR(avfilter_graph_parse(graph_.get(), filter_.c_str(), outputs_.get(), inputs_.get(), NULL));
	THROW_ON_FFMPEG_ERROR(avfilter_graph_config(graph_.get(), NULL));
	input_width_ = frame->width;
	input_height_ = frame->height;
	input_pixel_format_ = static_cast<AVPixelFormat>(frame->format);
	input_sar_ = frame->sample_aspect_ratio;
	input_time_base_ = frame->time_base;
	assert(av_buffersink_get_format(weak_sink) == output_pix_fmt_);
	DebugPrintLine(Common::DebugSeverity::debug, args.str());
	if (DebugSeverity() <= Common::DebugSeverity::info)
		DumpFilter(filter_, graph_.get());
}

bool VideoFilterBase::PushFrameFromBuffer()
{
	std::lock_guard<std::mutex> lock(frame_queue_mutex_);
	if (frame_buffer_.empty())
		return false; // buffer empty - normal case
	std::shared_ptr<AVFrame> &frame = frame_buffer_.front();
	if (frame->width != input_width_ ||
		frame->height != input_height_ ||
		frame->format != input_pixel_format_ ||
		av_cmp_q(frame->sample_aspect_ratio, input_sar_) != 0
		)
		Initialize(frame);
	if (FF_SUCCESS(av_buffersrc_write_frame(source_ctx_.get(), frame.get())))
	{
		DebugPrintLine(Common::DebugSeverity::trace, "PushFrameFromBuffer: " + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE));
		frame_buffer_.pop_front();
		return true;
	}
	DebugPrintLine(Common::DebugSeverity::warning, "av_buffersrc_write_frame failded for" + std::to_string(static_cast<float>(FrameTime(frame)) / AV_TIME_BASE));
	return false;
}


bool VideoFilterBase::IsInitialized() const
{
	return !!graph_;
}

void VideoFilterBase::Clear() 
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