#include "../pch.h"
#include "FFStreamOutput.h"
#include "FFStreamOutputParams.h"
#include "../Core/Channel.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "OutputFormat.h"

namespace TVPlayR {
	namespace FFmpeg {

		struct FFStreamOutput::implementation : Common::DebugTarget
		{
			const FFStreamOutputParams params_;
			AVDictionary* options_ = nullptr;
			OutputFormat output_format_;
			Common::Executor executor_;
			Core::VideoFormat format_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			AVPixelFormat pixel_format_;
			std::shared_ptr<AVFrame> last_video_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			int64_t video_frames_pushed_ = 0LL;
			int64_t audio_samples_pushed_ = 0LL;
			int64_t last_video_time_ = 0LL;
			std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> format_context_;
			std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> video_codec_ctx_;
			std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> audio_codec_ctx_;
			
			implementation(const FFStreamOutputParams& params)
				: Common::DebugTarget(false, "Stream output: " + params.Address)
				, output_format_(params.Address)
				, params_(params)
				, executor_("Stream output: " + params.Address)
				, format_(Core::VideoFormatType::invalid)
				, options_(ReadOptions(params.Options))
			{

			}
			
			~implementation()
			{
			}

			void Tick() 
			{

			}

			int AudioSamplesRequired()
			{
				int64_t samples_required = av_rescale(video_frames_pushed_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_pushed_;
				return static_cast<int>(samples_required);
			}

			AVDictionary* ReadOptions(const std::string& params) {
				AVDictionary* result = nullptr;
				if (av_dict_parse_string(&result, params.c_str(), "=", ",", 0))
					DebugPrintLine("ReadOptions failed for: " + params);
				return result;
			}

			AVFormatContext* CreateOutputFormatContext()
			{
				AVOutputFormat* format = NULL;
				const std::string& file_name = params_.Address;
				if (file_name.find("rtmp://") == 0)
					format = av_guess_format("flv", NULL, NULL);
				else
					format = av_guess_format("mpegts", NULL, NULL);

				if (!format)
					THROW_EXCEPTION("Could not find output format.");
				AVFormatContext* ctx = nullptr;
				THROW_ON_FFMPEG_ERROR(avformat_alloc_output_context2(&ctx, format, NULL, file_name.c_str()));
				return ctx;
			}

			void AddVideoStream()
			{
				const AVCodec* encoder = avcodec_find_encoder_by_name(params_.VideoCodec.c_str());
				if (!encoder)
					THROW_EXCEPTION("Encoder not found.");
				video_codec_ctx_ = std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>(avcodec_alloc_context3(encoder), [](AVCodecContext* ctx) { avcodec_free_context(&ctx); });

				video_codec_ctx_->codec_id = encoder->id;
				video_codec_ctx_->codec_type = AVMEDIA_TYPE_VIDEO;
				video_codec_ctx_->width = format_.width();
				video_codec_ctx_->height = format_.width();
				video_codec_ctx_->time_base = av_make_q(90000, 1);
				video_codec_ctx_->framerate = format_.FrameRate().av();
				video_codec_ctx_->max_b_frames = 0; // b-frames not supported by default.
				video_codec_ctx_->bit_rate = params_.VideoBitrate;

				if (format_.interlaced())
					video_codec_ctx_->flags |= (AV_CODEC_FLAG_INTERLACED_ME | AV_CODEC_FLAG_INTERLACED_DCT);

				if (video_codec_ctx_->codec_id == AV_CODEC_ID_PRORES)
				{
					video_codec_ctx_->bit_rate = video_codec_ctx_->width < 1280 ? 63 * 1000000 : 220 * 1000000;
					video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV422P10;
				}
				else if (video_codec_ctx_->codec_id == AV_CODEC_ID_DNXHD)
				{
					if (video_codec_ctx_->width < 1280 || video_codec_ctx_->height < 720)
						THROW_EXCEPTION("Unsupported video dimensions.");
					video_codec_ctx_->bit_rate = 220 * 1000000;
					video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV422P;
				}
				else if (video_codec_ctx_->codec_id == AV_CODEC_ID_DVVIDEO)
				{
					video_codec_ctx_->width = video_codec_ctx_->height == 1280 ? 960 : video_codec_ctx_->width;
					if (format_.type() == Core::VideoFormatType::ntsc || format_.type() == Core::VideoFormatType::ntsc_fha)
						video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV411P;
					else if (format_.type() == Core::VideoFormatType::pal || format_.type() == Core::VideoFormatType::pal_fha)
						video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
					else // dv50
						video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV422P;
					if (format_.FrameRate().Denominator() == 1001)
						video_codec_ctx_->width = video_codec_ctx_->height == 1080 ? 1280 : video_codec_ctx_->width;
					else
						video_codec_ctx_->width = video_codec_ctx_->height == 1080 ? 1440 : video_codec_ctx_->width;
				}
				else if (video_codec_ctx_->codec_id == AV_CODEC_ID_H264)
				{
					video_codec_ctx_->gop_size = 30;
					video_codec_ctx_->max_b_frames = 2;
					if (strcmp(video_codec_ctx_->codec->name, "libx264") == 0)
						if (av_dict_set(&options_, "preset", "veryfast", AV_DICT_DONT_OVERWRITE) < 0)
							DebugPrintLine("Setting preset for libx264 failed.");
				}
				else if (video_codec_ctx_->codec_id == AV_CODEC_ID_QTRLE)
				{
					video_codec_ctx_->pix_fmt = AV_PIX_FMT_ARGB;
				}

				if (format_context_->flags & AVFMT_GLOBALHEADER)
					video_codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
				video_codec_ctx_->sample_aspect_ratio = format_.SampleAspectRatio().av();
				if (video_codec_ctx_->pix_fmt == AV_PIX_FMT_NONE)
					video_codec_ctx_->pix_fmt = pixel_format_;

				THROW_ON_FFMPEG_ERROR(avcodec_open2(video_codec_ctx_.get(), encoder, &options_));
		
				AVStream* video_stream = avformat_new_stream(format_context_.get(), NULL);
				if (!video_stream)
					THROW_EXCEPTION("Could not allocate video-stream.");

				THROW_ON_FFMPEG_ERROR(avcodec_parameters_from_context(video_stream->codecpar, video_codec_ctx_.get()));

				video_stream->metadata = ReadOptions(params_.VideoMetadata);
				video_stream->id = params_.VideoStreamId;
				video_stream->sample_aspect_ratio = format_.SampleAspectRatio().av();
				video_stream->time_base = av_make_q(90000, 1);
				video_stream->avg_frame_rate = format_.FrameRate().av();
			}

			bool AssignToChannel(Core::Channel& channel)
			{
				return executor_.invoke([&]
					{
						if (format_.type() != Core::VideoFormatType::invalid)
							return false;
						format_ = channel.Format();
						audio_sample_rate_ = channel.AudioSampleRate();
						audio_channels_count_ = channel.AudioChannelsCount();
						last_video_ = FFmpeg::CreateEmptyVideoFrame(format_, channel.PixelFormat());
						pixel_format_ = Core::PixelFormatToFFmpegFormat(channel.PixelFormat());
						video_frames_pushed_ = 0LL;
						audio_samples_pushed_ = 0LL;
						last_video_time_ = 0LL;
						format_context_ = std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>>(CreateOutputFormatContext(), [this](AVFormatContext* ctx)
							{
								if (!(ctx->oformat->flags & AVFMT_NOFILE))
									if (avio_close(ctx->pb))
										DebugPrintLine("avio_close failed when finalizing format context");
								avformat_free_context(ctx);
							});
						if (format_context_)
							AddVideoStream();
						
						executor_.begin_invoke([this] { Tick(); }); // first frame
						return true;
					});
			}

			void ReleaseChannel()
			{
				executor_.invoke([this]
					{
						format_ = Core::VideoFormatType::invalid;
						video_codec_ctx_.reset();
						format_context_.reset();

					});
			}

			void Push(FFmpeg::AVSync& sync)
			{
				buffer_.try_add(sync);			
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { frame_requested_callback_ = frame_requested_callback; });
			}

		};

		FFStreamOutput::FFStreamOutput(const FFStreamOutputParams& params)
			: impl_(std::make_unique<implementation>(params))
		{ }

		FFStreamOutput::~FFStreamOutput() { }
		bool FFStreamOutput::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }
		void FFStreamOutput::ReleaseChannel() { impl_->ReleaseChannel(); }
		void FFStreamOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void FFStreamOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->SetFrameRequestedCallback(frame_requested_callback); }

		const FFStreamOutputParams& FFStreamOutput::GetStreamOutputParams() { return impl_->params_; }

	}
}

