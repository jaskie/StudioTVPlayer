#include "../pch.h"
#include "VideoOverlayFilter.h"
#include "../Core/VideoFormat.h"
#include "../Core/AVSync.h"
#include "../Common/Debug.h"
#include "FilterBase.h"
#include "FFmpegUtils.h"
#include "FFmpegBufferedInput.h"

namespace TVPlayR {
	namespace FFmpeg {
		struct VideoOverlayFilter::implementation : private FilterBase, private Common::DebugTarget
		{
			const Core::VideoFormat& format_;
			const AVPixelFormat output_pixel_format_;
			const AVRational output_time_base_;
			AVFilterContext* source_ctx_ = NULL;
			AVFilterContext* overlay_ctx_ = NULL;
			AVFilterContext* sink_ctx_ = NULL;
			std::shared_ptr<FFmpegInput> overlay_;
			std::int64_t current_frame_;

			implementation(const Core::VideoFormat& format, AVPixelFormat output_pixel_format)
				: Common::DebugTarget(Common::DebugSeverity::debug, "VideoOverlayFilter")
				, format_(format)
				, output_pixel_format_(output_pixel_format)
				, output_time_base_(format.FrameRate().invert().av())
				, current_frame_(0LL)
			{
			}

			Core::AVSync Transform(Core::AVSync& sync)
			{
				auto& video = sync.Video;
				video = FFmpeg::CloneFrame(video);
				video->pts = current_frame_++;

				return Core::AVSync(sync.Audio, video, sync.TimeInfo);
			}

			std::string GetFilterString()
			{
				return "overlay";
			}

			void Flush()
			{
				is_flushed_ = true;
			}

			AVRational OutputTimeBase() const
			{
				return output_time_base_;
			}

			std::shared_ptr<AVFrame> Pull()
			{
				return nullptr;
			}

			void CreateFilter(AVPixelFormat input_pixel_format)
			{
				graph_.reset(avfilter_graph_alloc());
				AVFilterInOut* outputs = avfilter_inout_alloc();
				AVFilterInOut* inputs = avfilter_inout_alloc(); // main input
				inputs->next = avfilter_inout_alloc(); // overlay input
				try
				{
					const AVFilter* buffersrc = avfilter_get_by_name("buffer");
					const AVFilter* buffersink = avfilter_get_by_name("buffersink");
					std::stringstream args;
					AVRational frame_rate = format_.FrameRate().av();
					AVRational sar = format_.SampleAspectRatio().av();
					args << "video_size=" << format_.width() << "x" << format_.height()
						<< ":pix_fmt=" << input_pixel_format
						<< ":time_base=" << frame_rate.den << "/" << frame_rate.num
						<< ":pixel_aspect=" << sar.num << "/" << sar.den;
					THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&source_ctx_, buffersrc, "main", args.str().c_str(), NULL, graph_.get()));
					enum AVPixelFormat output_pix_fmts[] = { output_pixel_format_, AV_PIX_FMT_NONE };
					THROW_ON_FFMPEG_ERROR(avfilter_graph_create_filter(&sink_ctx_, buffersink, "vout", NULL, NULL, graph_.get()));
					THROW_ON_FFMPEG_ERROR(av_opt_set_int_list(sink_ctx_, "output_pix_fmts", output_pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN));

					inputs->name = av_strdup("main");
					inputs->filter_ctx = source_ctx_;
					inputs->pad_idx = 0;
					inputs->next->name = av_strdup("overlay");
					inputs->next->filter_ctx = overlay_ctx_;
					inputs->next->pad_idx = 1;
					inputs->next->next = NULL;

					outputs->name = av_strdup("out");
					outputs->filter_ctx = sink_ctx_;
					outputs->pad_idx = 0;
					outputs->next = NULL;
					std::string filter = GetFilterString();
					THROW_ON_FFMPEG_ERROR(avfilter_graph_parse(graph_.get(), filter.c_str(), outputs, inputs, NULL));
					THROW_ON_FFMPEG_ERROR(avfilter_graph_config(graph_.get(), NULL));
					//input_width_ = input_width;
					//input_height_ = input_height;
					//input_pixel_format_ = static_cast<AVPixelFormat>(input_pixel_format);
					//input_sar_ = input_sar;
					//DebugPrintLine(Common::DebugSeverity::debug, args.str());
					if (DebugSeverity() <= Common::DebugSeverity::info)
						DumpFilter(filter, graph_.get());
				}
				catch (const std::exception& e)
				{
					avfilter_inout_free(&outputs);
					avfilter_inout_free(&inputs);
					throw e;
				}
			}


		};

		VideoOverlayFilter::VideoOverlayFilter(const Core::VideoFormat& format, AVPixelFormat output_pixel_format)
			: impl_(std::make_unique<implementation>(format, output_pixel_format))
		{ }

		VideoOverlayFilter::~VideoOverlayFilter() { }

		Core::AVSync VideoOverlayFilter::Transform(Core::AVSync& sync) { return impl_->Transform(sync); }

	}
}