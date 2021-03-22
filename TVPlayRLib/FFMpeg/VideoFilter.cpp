#include "../pch.h"
#include "VideoFilter.h"

#undef DEBUG


namespace TVPlayR {
	namespace FFmpeg {
		class VideoFilter::implementation
		{
		private:
			AVFilterContext* source_ctx_ = NULL;
			AVFilterContext* sink_ctx_ = NULL;
			AVFilterGraphPtr graph_;
			const AVPixelFormat output_pix_fmt_;
			bool is_flushed_ = false;
			bool is_eof_ = false;

		public:
			implementation(AVPixelFormat output_pix_fmt)
				: output_pix_fmt_(output_pix_fmt)
				, graph_(nullptr, [](AVFilterGraph * g) { avfilter_graph_free(&g); })
			{
			}

			bool Push(std::shared_ptr<AVFrame> frame)
			{
				return av_buffersrc_write_frame(source_ctx_, frame.get()) >= 0;
			}

			std::shared_ptr<AVFrame> Pull()
			{
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
					return nullptr;
				case AVERROR(EINVAL):
					return nullptr;
				}
				if (FF(ret))
				{
					//if (frame->best_effort_timestamp == AV_NOPTS_VALUE)
					//	frame->best_effort_timestamp = frame->pts;
					//frame->pts = av_rescale_q(frame->best_effort_timestamp, input_time_base_, av_buffersink_get_time_base(sink_ctx_));
#ifdef DEBUG
					auto tb = av_buffersink_get_time_base(sink_ctx_);
					OutputDebugStringA(("Pulled from VideoFilter: " + std::to_string(PtsToTime(frame->pts, tb) / 1000) + "\n").c_str());
#endif
					return frame;
				}
				return nullptr;
			}

			int OutputWidth()
			{
				assert(sink_ctx_);
				return av_buffersink_get_w(sink_ctx_);
			}

			int OutputHeight()
			{
				assert(sink_ctx_);
				return av_buffersink_get_h(sink_ctx_);
			}

			AVRational OutputSampleAspectRatio()
			{
				assert(sink_ctx_);
				return av_buffersink_get_sample_aspect_ratio(sink_ctx_);
			}

			AVRational OutputFrameRate() const
			{
				assert(sink_ctx_);
				AVRational frame_rate = av_buffersink_get_frame_rate(sink_ctx_);
				if (frame_rate.num > 0)
					return frame_rate;
			}

			AVPixelFormat OutputPixelFormat()
			{
				assert(sink_ctx_);
				return static_cast<AVPixelFormat>(av_buffersink_get_format(sink_ctx_));
			}

			AVRational OutputTimeBase() const
			{
				assert(sink_ctx_);
				return av_buffersink_get_time_base(sink_ctx_);
			}

			void Flush()
			{
				if (source_ctx_)
					av_buffersrc_write_frame(source_ctx_, NULL);
				is_flushed_ = true;
			}

			void CreateFilterChain(std::shared_ptr<AVFrame> frame, const AVRational& input_time_base, const std::string& filter_str)
			{
				source_ctx_ = NULL;
				sink_ctx_ = NULL;
				is_eof_ = false;
				is_flushed_ = false;

				graph_.reset(avfilter_graph_alloc());
				AVFilterInOut * inputs = avfilter_inout_alloc();
				AVFilterInOut * outputs = avfilter_inout_alloc();
				AVBufferSinkParams * buffersink_params = av_buffersink_params_alloc();
				try
				{
					const AVFilter * buffersrc = avfilter_get_by_name("buffer");
					const AVFilter * buffersink = avfilter_get_by_name("buffersink");
					char args[512];
					snprintf(args, sizeof(args),
						"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
						frame->width, frame->height, frame->format,
						input_time_base.num, input_time_base.den,
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
#ifdef DEBUG
					OutputDebugStringA(args);
					dump_filter(filter_str_, graph_);
#endif // DEBUG
				}
				catch (const std::exception& e)
				{
					avfilter_inout_free(&inputs);
					avfilter_inout_free(&outputs);
					throw e;
				}
				av_free(buffersink_params);
			}

			bool IsFlushed() const { return is_flushed_; }

			bool IsInitializded() const { return !!graph_; }

			bool IsEof() const { return is_eof_; }

			void Reset()
			{
				source_ctx_ = NULL;
				sink_ctx_ = NULL;
				is_eof_ = false;
				is_flushed_ = false;
				graph_.reset();
			}

		};

VideoFilter::VideoFilter(AVPixelFormat output_pix_fmt)
	: impl_(std::make_unique<implementation>(output_pix_fmt))
{ }
VideoFilter::~VideoFilter() { }
bool VideoFilter::Push(std::shared_ptr<AVFrame> frame) { return impl_->Push(frame); }
std::shared_ptr<AVFrame> VideoFilter::Pull() { return impl_->Pull(); }
int VideoFilter::OutputWidth() { return impl_->OutputWidth(); }
int VideoFilter::OutputHeight() { return impl_->OutputHeight(); }
AVRational VideoFilter::OutputSampleAspectRatio() { return impl_->OutputSampleAspectRatio(); }
AVPixelFormat VideoFilter::GetOutputPixelFormat() { return impl_->OutputPixelFormat(); }
AVRational VideoFilter::OutputTimeBase() const { return impl_->OutputTimeBase(); }
void VideoFilter::Flush() { return impl_->Flush(); }
void VideoFilter::CreateFilterChain(std::shared_ptr<AVFrame> frame, const AVRational& input_time_base, const std::string& filter_str) { impl_->CreateFilterChain(frame, input_time_base, filter_str); }
bool VideoFilter::IsInitialized() const { return impl_->IsInitializded(); }
bool VideoFilter::IsEof() const { return impl_->IsEof(); }
void VideoFilter::Reset() { impl_->Reset(); }
bool VideoFilter::IsFlushed() const { return impl_->IsFlushed(); }
}}