#include "../pch.h"
#include "FFStreamOutput.h"
#include "../Core/Channel.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "OutputFormat.h"
#include "Encoder.h"

namespace TVPlayR {
	namespace FFmpeg {

		struct FFStreamOutput::implementation : Common::DebugTarget
		{
			const FFStreamOutputParams& params_;
			AVDictionary* options_ = nullptr;
			Common::Executor executor_;
			Core::VideoFormat format_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			OutputFormat output_format_;
			Encoder video_encoder_;
			Encoder audio_encoder_;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			std::shared_ptr<AVFrame> last_video_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			int64_t video_frames_pushed_ = 0LL;
			int64_t audio_samples_pushed_ = 0LL;
			int64_t last_video_time_ = 0LL;

			implementation(const Core::Channel& channel, const FFStreamOutputParams& params)
				: Common::DebugTarget(false, "Stream output: " + params.Address)
				, params_(params)
				, format_(channel.Format())
				, audio_sample_rate_(channel.AudioSampleRate())
				, audio_channels_count_(channel.AudioChannelsCount())
				, executor_("Stream output: " + params.Address)
				, options_(ReadOptions(params.Options))
				, output_format_(params.Address)
				, audio_encoder_(output_format_, params.AudioCodec, params.AudioBitrate, channel.AudioSampleFormat(), channel.AudioSampleRate(), channel.AudioChannelsCount(), &options_, params.AudioMetadata, params.AudioStreamId)
				, video_encoder_(output_format_, params.VideoCodec, params.VideoBitrate, channel.Format(), channel.PixelFormat(), &options_, params.VideoMetadata, params.VideoStreamId)
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

			/*
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
			*/

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
			: params_(params)
		{ }

		FFStreamOutput::~FFStreamOutput() { }
		bool FFStreamOutput::AssignToChannel(const Core::Channel& channel)
		{
			if (impl_)
				return false;
			impl_ = std::make_unique<implementation>(channel, params_);
			return true;
		}
		void FFStreamOutput::ReleaseChannel() { impl_.reset(); }
		void FFStreamOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void FFStreamOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->SetFrameRequestedCallback(frame_requested_callback); }
		const FFStreamOutputParams& FFStreamOutput::GetStreamOutputParams() { return params_; }
	}
}

