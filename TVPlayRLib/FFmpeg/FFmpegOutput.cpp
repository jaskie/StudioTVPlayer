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
			std::unique_ptr<Encoder> video_encoder_;
			Encoder audio_encoder_;
			std::unique_ptr<SwScale> video_scaler_;
			std::unique_ptr<OutputVideoFilter> video_filter_;
			SwResample audio_resampler_;
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
				, audio_encoder_(output_format_, params.AudioCodec, params.AudioBitrate, channel.AudioSampleRate(), channel.AudioChannelsCount(), &options_, params.AudioMetadata, params.AudioStreamId)
				, audio_resampler_(channel.AudioChannelsCount(), channel.AudioSampleRate(), channel.AudioSampleFormat(), channel.AudioChannelsCount(), audio_encoder_.SampleRate(), static_cast<AVSampleFormat>(audio_encoder_.Format()))
				, buffer_(2)
				, executor_("Stream output: " + params.Url)
			{


				const AVCodec* video_encoder = avcodec_find_encoder_by_name(params.VideoCodec.c_str());

				if (params.OutputFilter.empty())
					video_scaler_ = std::make_unique<SwScale>(format_.width(), format_.height(), src_pixel_format_, format_.width(), format_.height(), video_encoder->pix_fmts[0]);
				else
					video_filter_ = std::make_unique<OutputVideoFilter>(channel, params.OutputFilter, video_encoder->pix_fmts[0]);

				video_encoder_ = std::make_unique<Encoder>(output_format_, video_encoder, params.VideoBitrate, format_, &options_, params.VideoMetadata, params.VideoStreamId);
				
				if (options_)
				{
					char* unused_options;
					if (av_dict_count(options_) > 0 && av_dict_get_string(options_, &unused_options, '=', ',') >= 0 && unused_options)
					{
						DebugPrintLine("Following options were not parsed: " + std::string(unused_options));
						delete(unused_options);
					}
					av_dict_free(&options_);
				}
				executor_.begin_invoke([this]
				{
					auto time = clock_.now().time_since_epoch().count();

					output_format_.Initialize(params_.OutputMetadata);
#ifdef DEBUG
					av_dump_format(output_format_.Ctx(), 0, params_.Url.c_str(), true);
#endif
				});
				
			}

			~implementation()
			{
				executor_.invoke([this]
				{
					video_encoder_->Flush();
					audio_encoder_.Flush();
					PushPackets(*video_encoder_);
					PushPackets(audio_encoder_);
				});
			}
			
			void Tick() 
			{
				executor_.begin_invoke([this] {
					AVSync sync;
					if (buffer_.try_take(sync) == TVPlayR::Common::BlockingCollectionStatus::Ok)
					{
						video_encoder_->Push(video_scaler_->Scale(sync.Video));
						audio_encoder_.Push(audio_resampler_.Resample(sync.Audio));
						last_video_ = sync.Video;
						PushPackets(*video_encoder_);
						PushPackets(audio_encoder_);
					}
					else
					{

					}
					
				});
			}

			void PushPackets(Encoder& encoder)
			{
				while (true)
				{
					auto packet = encoder.Pull();
					if (!packet)
						return;
					output_format_.Push(packet);
				}
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

