#include "../pch.h"
#include "FFmpegOutput.h"
#include "../Core/Channel.h"
#include "../PixelFormat.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "OutputFormat.h"
#include "Encoder.h"
#include "SwScale.h"
#include "SwResample.h"
#include "OutputVideoFilter.h"
#include <chrono>

namespace TVPlayR {
	namespace FFmpeg {

		struct FFmpegOutput::implementation : Common::DebugTarget
		{
			const FFOutputParams& params_;
			AVDictionary* options_ = nullptr;
			Core::VideoFormat format_;
			const AVPixelFormat src_pixel_format_;
			int audio_channels_count_;
			int audio_sample_rate_;
			OutputFormat output_format_;
			const AVCodec* video_codec_;
			const AVCodec* audio_codec_;
			std::unique_ptr<SwScale> video_scaler_;
			std::unique_ptr<OutputVideoFilter> video_filter_;
			SwResample audio_resampler_;
			std::unique_ptr<Encoder> video_encoder_;
			std::unique_ptr<Encoder> audio_encoder_;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			std::shared_ptr<AVFrame> last_video_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			int64_t video_frames_pushed_ = 0LL;
			int64_t audio_samples_pushed_ = 0LL;
			int64_t last_video_time_ = 0LL;
			std::chrono::steady_clock clock_;
			Common::Executor executor_;

			implementation(const Core::Channel& channel, const FFOutputParams& params)
				: Common::DebugTarget(true, "Stream output: " + params.Url)
				, params_(params)
				, format_(channel.Format())
				, src_pixel_format_(PixelFormatToFFmpegFormat(channel.PixelFormat()))
				, audio_sample_rate_(channel.AudioSampleRate())
				, audio_channels_count_(channel.AudioChannelsCount())
				, options_(ReadOptions(params.Options))
				, output_format_(params.Url, options_)
				, audio_resampler_(channel.AudioChannelsCount(), channel.AudioSampleRate(), channel.AudioSampleFormat(), channel.AudioChannelsCount(), channel.AudioSampleRate(), static_cast<AVSampleFormat>(audio_codec_->sample_fmts[0]))
				, buffer_(2)
				, video_codec_(avcodec_find_encoder_by_name(params.VideoCodec.c_str()))
				, audio_codec_(avcodec_find_encoder_by_name(params.AudioCodec.c_str()))
				, executor_("Stream output: " + params.Url)
			{

				if (params.VideoFilter.empty())
					video_scaler_ = std::make_unique<SwScale>(format_.width(), format_.height(), src_pixel_format_, format_.width(), format_.height(), video_codec_->pix_fmts[0]);
				else
					video_filter_ = std::make_unique<OutputVideoFilter>(channel.Format().FrameRate().av(), params.VideoFilter, video_codec_->pix_fmts[0]);

				executor_.begin_invoke([this]
				{
					auto time = clock_.now().time_since_epoch().count();
				});
				
			}

			~implementation()
			{
				executor_.invoke([this]
				{
					video_encoder_->Flush();
					audio_encoder_->Flush();
					PushPackets(video_encoder_);
					PushPackets(audio_encoder_);
				});
			}
			
			void Tick() 
			{
				executor_.begin_invoke([this] {
					AVSync sync;
					if (buffer_.try_take(sync) == TVPlayR::Common::BlockingCollectionStatus::Ok)
					{
						std::shared_ptr<AVFrame> video;
						if (video_filter_)
						{
							video_filter_->Push(sync.Video);
							while (video = video_filter_->Pull())
							{
								if (!video_encoder_)
								{
									video_encoder_ = std::make_unique<Encoder>(output_format_, video_codec_, params_.VideoBitrate, video, video_filter_->OutputTimeBase(), video_filter_->OutputFrameRate(), &options_, params_.VideoMetadata, params_.VideoStreamId);
									InitializeOuputIfPossible();
								}
								video_encoder_->Push(video);
								PushPackets(video_encoder_);
							}
						} 
						else if (video_scaler_)
						{
							auto video = video_scaler_->Scale(sync.Video);
							if (!video_encoder_)
							{
								video_encoder_ = std::make_unique<Encoder>(output_format_, video_codec_, params_.VideoBitrate, video, format_.FrameRate().invert().av(), format_.FrameRate().av(), &options_, params_.VideoMetadata, params_.VideoStreamId);
								InitializeOuputIfPossible();
							}
							video_encoder_->Push(video_scaler_->Scale(sync.Video));
							PushPackets(video_encoder_);
						}
						last_video_ = sync.Video;
						if (!audio_encoder_)
						{
							audio_encoder_ = std::make_unique<Encoder>(output_format_, audio_codec_, params_.AudioBitrate, audio_resampler_.OutputSampleRate(), audio_resampler_.OutputChannelCount(), &options_, params_.AudioMetadata, params_.AudioStreamId);
							InitializeOuputIfPossible();
						}
						audio_encoder_->Push(audio_resampler_.Resample(sync.Audio));
						PushPackets(audio_encoder_);
					}
					else
					{

					}
					
				});
			}

			void InitializeOuputIfPossible()
			{
				if (video_encoder_ && audio_encoder_)
					output_format_.Initialize(params_.OutputMetadata);
#ifdef DEBUG
				av_dump_format(output_format_.Ctx(), 0, params_.Url.c_str(), true);
#endif
				if (options_)
				{
					char* unused_options;
					if (av_dict_count(options_) > 0 && av_dict_get_string(options_, &unused_options, '=', ',') >= 0 && unused_options)
					{
						DebugPrintLine("Following options were not parsed: " + std::string(unused_options));
						av_free(unused_options);
					}
					av_dict_free(&options_);
				}
			}

			void PushPackets(std::unique_ptr<Encoder>& encoder)
			{
				while (auto packet = encoder->Pull())
					output_format_.Push(packet);
			}

			int AudioSamplesRequired()
			{
				int64_t samples_required = av_rescale(video_frames_pushed_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_pushed_;
				return static_cast<int>(samples_required);
			}

			void Push(FFmpeg::AVSync& sync)
			{
				buffer_.try_add(sync);		
				Tick();
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { frame_requested_callback_ = frame_requested_callback; });
			}

		};

		FFmpegOutput::FFmpegOutput(const FFOutputParams params)
			: params_(params)
		{ }

		FFmpegOutput::~FFmpegOutput() { }
		bool FFmpegOutput::AssignToChannel(const Core::Channel& channel)
		{
			if (impl_)
				return false;
			impl_ = std::make_unique<implementation>(channel, params_);
			return true;
		}
		void FFmpegOutput::ReleaseChannel() { impl_.reset(); }
		void FFmpegOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void FFmpegOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->SetFrameRequestedCallback(frame_requested_callback); }
		const FFOutputParams& FFmpegOutput::GetStreamOutputParams() { return params_; }
	}
}

