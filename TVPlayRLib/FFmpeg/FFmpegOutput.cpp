#include "../pch.h"
#include <chrono>
#include "FFmpegOutput.h"
#include "../Core/Channel.h"
#include "../PixelFormat.h"
#include "../Core/VideoFormat.h"
#include "../Core/OverlayBase.h"
#include "OutputFormat.h"
#include "Encoder.h"
#include "SwScale.h"
#include "SwResample.h"
#include "OutputVideoFilter.h"
#include "AVSync.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

		typedef std::chrono::steady_clock clock;

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
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			std::int64_t video_frames_requested_ = 0LL;
			std::int64_t audio_samples_requested_ = 0LL;
			std::int64_t video_frames_pushed_ = 0LL;
			std::int64_t audio_samples_pushed_ = 0LL;
			std::int64_t last_video_time_ = 0LL;
			clock::time_point stream_start_time_;
			Common::Executor executor_;

			implementation(const Core::Channel& channel, const FFOutputParams& params, FRAME_REQUESTED_CALLBACK frame_requested_callback)
				: Common::DebugTarget(false, "FFmpeg output: " + params.Url)
				, params_(params)
				, frame_requested_callback_(frame_requested_callback)
				, format_(channel.Format())
				, src_pixel_format_(PixelFormatToFFmpegFormat(channel.PixelFormat()))
				, audio_sample_rate_(channel.AudioSampleRate())
				, audio_channels_count_(channel.AudioChannelsCount())
				, options_(ReadOptions(params.Options))
				, output_format_(params.Url, options_)
				, audio_resampler_(channel.AudioChannelsCount(), channel.AudioSampleRate(), channel.AudioSampleFormat(), channel.AudioChannelsCount(), channel.AudioSampleRate(), static_cast<AVSampleFormat>(audio_codec_->sample_fmts[0]))
				, buffer_(6)
				, video_codec_(avcodec_find_encoder_by_name(params.VideoCodec.c_str()))
				, audio_codec_(avcodec_find_encoder_by_name(params.AudioCodec.c_str()))
				, executor_("Stream output: " + params.Url)
			{
				if (params.VideoFilter.empty())
					video_scaler_ = std::make_unique<SwScale>(format_.width(), format_.height(), src_pixel_format_, format_.width(), format_.height(), video_codec_->pix_fmts[0]);
				else
					video_filter_ = std::make_unique<OutputVideoFilter>(channel.Format().FrameRate().av(), params.VideoFilter, video_codec_->pix_fmts[0]);
				if (frame_requested_callback)
					executor_.begin_invoke([this] { InitializeFrameRequester(); });
			}

			~implementation()
			{
				executor_.invoke([this]
				{
					PushToEncoder(video_encoder_, nullptr);
					PushToEncoder(audio_encoder_, nullptr);
					output_format_.Flush();
				});
			}

			void InitializeFrameRequester()
			{
				assert(executor_.is_current());
				stream_start_time_ = clock::now();
				audio_samples_requested_ = 0LL;
				video_frames_requested_ = 0LL;
				while (video_frames_requested_ <= static_cast<std::int64_t>(buffer_.bounded_capacity() / 2))
					RequestFrameFromChannel();
				Tick();
			}
			
			void Tick()
			{
				assert(executor_.is_current());
				AVSync sync;
				if (buffer_.try_take(sync) == TVPlayR::Common::BlockingCollectionStatus::Ok)
				{
					for (auto& overlay : overlays_)
						sync = overlay->Transform(sync);
					if (video_filter_)
					{
						video_filter_->Push(sync.Video);
						while (auto video = video_filter_->Pull())
						{
							if (!video_encoder_)
							{
								video_encoder_ = std::make_unique<Encoder>(output_format_, video_codec_, params_.VideoBitrate, video, video_filter_->OutputTimeBase(), video_filter_->OutputFrameRate(), &options_, params_.VideoMetadata, params_.VideoStreamId);
								InitializeOuputIfPossible();
							}
							PushToEncoder(video_encoder_, video);
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
						PushToEncoder(video_encoder_, video);
					}
					if (!audio_encoder_)
					{
						audio_encoder_ = std::make_unique<Encoder>(output_format_, audio_codec_, params_.AudioBitrate, audio_resampler_.OutputSampleRate(), audio_resampler_.OutputChannelCount(), &options_, params_.AudioMetadata, params_.AudioStreamId);
						InitializeOuputIfPossible();
					}
					if (sync.Audio)
					{
						PushToEncoder(audio_encoder_, audio_resampler_.Resample(sync.Audio));
					}
				}
				else
					DebugPrintLine("Buffer didn't return frame");
				if (frame_requested_callback_)
				{
					RequestFrameFromChannel();
					if (!output_format_.IsFlushed())
						executor_.begin_invoke([this]
					{
						WaitForNextFrameTime();
						Tick();
					});
				}
			}

			void RequestFrameFromChannel()
			{
				assert(executor_.is_current());
				int audio_samples_required = static_cast<int>(av_rescale(video_frames_requested_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_requested_);
				frame_requested_callback_(audio_samples_required);
				video_frames_requested_++;
				audio_samples_requested_ += audio_samples_required;
			}

			void PushToEncoder(const std::unique_ptr<Encoder>& encoder, const std::shared_ptr<AVFrame>& frame)
			{
				assert(executor_.is_current());
				if (frame)
					encoder->Push(frame);
				else
					encoder->Flush();
				while (auto packet = encoder->Pull())
					output_format_.Push(packet);
			}

			void WaitForNextFrameTime()
			{
				assert(executor_.is_current());
				auto& frame_rate = format_.FrameRate();
				clock::time_point current_time = clock::now();
				clock::time_point next_frame_time = stream_start_time_ + clock::duration(clock::period::den * frame_rate.Denominator() * video_frames_requested_ / (clock::period::num * frame_rate.Numerator()));
				auto wait_time = next_frame_time - current_time;
				if (wait_time > clock::duration::zero())
				{
					DebugPrintLine("Waiting " + std::to_string(wait_time.count() / 1000000) + " ms");
					std::this_thread::sleep_for(wait_time);
				}
				else
					DebugPrintLine("Negative wait time");
			}

			void InitializeOuputIfPossible()
			{
				assert(executor_.is_current());
				if (!video_encoder_ || !audio_encoder_)
					return;
				output_format_.Initialize(params_.OutputMetadata);
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

			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				executor_.invoke([&]
					{
						overlays_.emplace_back(overlay);
					});
			}

			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				executor_.invoke([&]
					{
						overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
					});
			}

			void Push(FFmpeg::AVSync& sync)
			{
				std::shared_ptr<AVFrame> audio(CloneFrame(sync.Audio));
				std::shared_ptr<AVFrame> video(CloneFrame(sync.Video));
				audio->pts = audio_samples_pushed_;
				audio_samples_pushed_ += audio->nb_samples;
				video->pts = video_frames_pushed_;
				video_frames_pushed_++;
				if (buffer_.try_emplace(AVSync(audio, video, sync.Timecode)) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine("Frame dropped");
				executor_.begin_invoke([this]
				{
					if (frame_requested_callback_)
						return;
					Tick();
				});
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { 
					frame_requested_callback_ = frame_requested_callback; 
					if (frame_requested_callback)
						InitializeFrameRequester();
				});
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
			impl_ = std::make_unique<implementation>(channel, params_, frame_requested_callback_);
			for (auto& overlay : overlays_)
				impl_->AddOverlay(overlay);
			return true;
		}
		void FFmpegOutput::ReleaseChannel() { impl_.reset(); }

		void FFmpegOutput::AddOverlay(std::shared_ptr<Core::OverlayBase> overlay) 
		{ 
			overlays_.emplace_back(overlay);
			if (impl_)
				impl_->AddOverlay(overlay); 
		}

		void FFmpegOutput::RemoveOverlay(std::shared_ptr<Core::OverlayBase> overlay) 
		{
			overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
			if (impl_)
				impl_->RemoveOverlay(overlay); 
		}

		void FFmpegOutput::Push(FFmpeg::AVSync& sync)
		{
			if (impl_) 
				impl_->Push(sync);
		}
		void FFmpegOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) 
		{ 
			frame_requested_callback_ = frame_requested_callback;
			if (impl_)
				impl_->SetFrameRequestedCallback(frame_requested_callback); 
		}
		const FFOutputParams& FFmpegOutput::GetStreamOutputParams() { return params_; }
	}
}

