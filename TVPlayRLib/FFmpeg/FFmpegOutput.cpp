#include "../pch.h"
#include <chrono>
#include "FFmpegOutput.h"
#include "../Core/Player.h"
#include "../PixelFormat.h"
#include "../Core/VideoFormat.h"
#include "../Core/OverlayBase.h"
#include "OutputFormat.h"
#include "Encoder.h"
#include "SwScale.h"
#include "SwResample.h"
#include "OutputVideoFilter.h"
#include "../Core/AVSync.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {

		typedef std::chrono::steady_clock clock;

		struct FFmpegOutput::implementation : Common::DebugTarget
		{
			const FFOutputParams params_;
			AVDictionary* options_ = nullptr;
			Core::VideoFormat format_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			OutputFormat output_format_;
			const AVCodec* video_codec_;
			const AVCodec* audio_codec_;
			AVPixelFormat src_pixel_format_ = AVPixelFormat::AV_PIX_FMT_NONE;
			AVPixelFormat dest_pixel_format_ = AVPixelFormat::AV_PIX_FMT_NONE;
			std::unique_ptr<SwScale> video_scaler_;
			std::unique_ptr<OutputVideoFilter> video_filter_;
			std::unique_ptr<SwResample> audio_resampler_;
			std::unique_ptr<Encoder> video_encoder_;
			std::unique_ptr<Encoder> audio_encoder_;
			Common::BlockingCollection<Core::AVSync> buffer_;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			std::int64_t video_frames_requested_ = 0LL;
			std::int64_t audio_samples_requested_ = 0LL;
			std::int64_t video_frames_pushed_ = 0LL;
			std::int64_t audio_samples_pushed_ = 0LL;
			std::int64_t last_video_time_ = 0LL;
			clock::time_point stream_start_time_;
			Common::Executor output_executor_;

			// frame clock variables
			std::atomic_bool frame_clock_thread_is_running_;
			std::mutex frame_clock_mutex_;
			std::vector<Core::ClockTarget*> frame_clock_targets_;
			std::thread frame_clock_thread_;

			implementation(const FFOutputParams& params)
				: Common::DebugTarget(Common::DebugSeverity::info, "FFmpeg output: " + params.Url)
				, params_(params)
				, format_(Core::VideoFormatType::invalid)
				, options_(ReadOptions(params.Options))
				, output_format_(params.Url, params.OutputFormat, options_)
				, buffer_(6)
				, video_codec_(avcodec_find_encoder_by_name(params.VideoCodec.c_str()))
				, audio_codec_(avcodec_find_encoder_by_name(params.AudioCodec.c_str()))
				, output_executor_("FFmpegOutput: " + params.Url)
			{ }

			~implementation()
			{
				frame_clock_thread_is_running_ = false;
				output_executor_.invoke([this]
				{
					if (video_encoder_)
						PushToEncoder(video_encoder_, nullptr);
					if (audio_encoder_)
						PushToEncoder(audio_encoder_, nullptr);
					output_format_.Flush();
					format_ = Core::VideoFormatType::invalid;
				});
				frame_clock_thread_.join();
			}

#pragma region Frame clock

			void FrameClockTheradStart()
			{
#ifdef DEBUG
				Common::SetThreadName("FFmpegOutputClock");
#endif
				::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
				stream_start_time_ = clock::now();
				audio_samples_requested_ = 0LL;
				video_frames_requested_ = 0LL;
				while (frame_clock_thread_is_running_)
				{
					RequestNextFrame();
					WaitForNextFrameTime();
				}
			}

			void RequestNextFrame()
			{
				DebugPrintLine(Common::DebugSeverity::trace, "RequestNextFrame");
				int audio_samples_required = static_cast<int>(av_rescale(video_frames_requested_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_requested_);
				std::lock_guard<std::mutex> lock(frame_clock_mutex_);
				for (Core::ClockTarget* target : frame_clock_targets_)
					target->RequestFrame(audio_samples_required);
				video_frames_requested_++;
				audio_samples_requested_ += audio_samples_required;
			}

			void WaitForNextFrameTime()
			{
				auto& frame_rate = format_.FrameRate();
				clock::time_point current_time = clock::now();
				clock::time_point next_frame_time = stream_start_time_ + clock::duration(clock::period::den * frame_rate.Denominator() * video_frames_requested_ / (clock::period::num * frame_rate.Numerator()));
				auto wait_time = next_frame_time - current_time;
				if (wait_time > clock::duration::zero())
				{
					DebugPrintLine(Common::DebugSeverity::trace, "Waiting " + std::to_string(wait_time.count() / 1000000) + " ms");
					std::this_thread::sleep_for(wait_time);
				}
				else
					DebugPrintLine(Common::DebugSeverity::debug, "Negative wait time");
			}

#pragma endregion // Frame clock

			void Initialize(Core::VideoFormatType video_format, TVPlayR::PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate)
			{
				if (format_.type() != Core::VideoFormatType::invalid)
					THROW_EXCEPTION("FFmpegOutput: already initialized")
				format_ = video_format;
				src_pixel_format_ =  PixelFormatToFFmpegFormat(pixel_format);
				dest_pixel_format_ = FindBestOutputPixelFormat(video_codec_, params_.PixelFormat, src_pixel_format_);
				audio_channels_count_ = audio_channel_count;
				audio_sample_rate_ = audio_sample_rate;
				audio_resampler_ = std::make_unique<SwResample>(audio_channel_count, audio_sample_rate, AVSampleFormat::AV_SAMPLE_FMT_FLT, audio_channel_count, audio_sample_rate, static_cast<AVSampleFormat>(audio_codec_->sample_fmts[0]));
				if (params_.VideoFilter.empty())
					video_scaler_ = std::make_unique<SwScale>(format_.width(), format_.height(), src_pixel_format_, format_.width(), format_.height(), dest_pixel_format_);
				else
					video_filter_ = std::make_unique<OutputVideoFilter>(format_.FrameRate().av(), params_.VideoFilter, dest_pixel_format_);

				frame_clock_thread_is_running_ = true;
				frame_clock_thread_ = std::thread(&implementation::FrameClockTheradStart, this);
			}

			void InitializeClock()
			{
				assert(output_executor_.is_current());
				ProcessFrameFromBuffer();
			}

			void ProcessFrameFromBuffer()
			{
				assert(output_executor_.is_current());
				Core::AVSync sync;
				if (buffer_.try_take(sync) == TVPlayR::Common::BlockingCollectionStatus::Ok)
				{
					for (auto& overlay : overlays_)
						sync = overlay->Transform(sync);
					std::shared_ptr<AVFrame> processed_video;
					if (video_filter_)
					{
						video_filter_->Push(sync.Video);
						processed_video = video_filter_->Pull();
						if (!video_encoder_ && processed_video)
						{
							AVRational frame_rate = video_filter_->OutputFrameRate();
							if (frame_rate.num == 0)
								frame_rate = format_.FrameRate().av();
							InitializeVideoEncoderAndOutput(processed_video, video_filter_->OutputTimeBase(), frame_rate);
						}
					}
					else if (video_scaler_)
					{
						processed_video = video_scaler_->Scale(sync.Video);
						if (!video_encoder_ && processed_video)
							InitializeVideoEncoderAndOutput(processed_video, format_.FrameRate().invert().av(), format_.FrameRate().av());
					}
					if (!audio_encoder_)
					{
						audio_encoder_ = std::make_unique<Encoder>(output_format_, audio_codec_, params_.AudioBitrate, audio_resampler_->OutputSampleRate(), audio_resampler_->OutputChannelLayout(), &options_, params_.AudioMetadata, params_.AudioStreamId);
						InitializeOuputIfPossible();
					}
					if (video_encoder_ && processed_video)
						PushToEncoder(video_encoder_, processed_video);
					if (sync.Audio)
						PushToEncoder(audio_encoder_, audio_resampler_->Resample(sync.Audio));
				}
				else
					DebugPrintLine(Common::DebugSeverity::info, "Buffer didn't return frame");
			}

			void PushToEncoder(const std::unique_ptr<Encoder>& encoder, const std::shared_ptr<AVFrame>& frame)
			{
				assert(output_executor_.is_current());
				if (frame)
					encoder->Push(frame);
				else
					encoder->Flush();
				while (auto packet = encoder->Pull())
					output_format_.Push(packet);
			}

			void InitializeVideoEncoderAndOutput(const std::shared_ptr<AVFrame>& frame, AVRational time_base, AVRational frame_rate)
			{
				video_encoder_ = std::make_unique<Encoder>(output_format_, video_codec_, params_.VideoBitrate, dest_pixel_format_, frame, time_base, frame_rate, &options_, params_.VideoMetadata, params_.VideoStreamId);
				InitializeOuputIfPossible();
			}

			void InitializeOuputIfPossible()
			{
				assert(output_executor_.is_current());
				if (!video_encoder_ || !audio_encoder_)
					return;
				output_format_.Initialize(params_.OutputMetadata);
				if (options_)
				{
					char* unused_options;
					if (av_dict_count(options_) > 0 && av_dict_get_string(options_, &unused_options, '=', ',') >= 0 && unused_options)
					{
						DebugPrintLine(Common::DebugSeverity::error, "Following options were not parsed: " + std::string(unused_options));
						av_free(unused_options);
					}
					av_dict_free(&options_);
				}
			}

			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				output_executor_.invoke([=]
					{
						overlays_.emplace_back(overlay);
					});
			}

			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				output_executor_.invoke([=]
					{
						overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
					});
			}

			void Push(Core::AVSync& sync)
			{
				DebugPrintLine(Common::DebugSeverity::trace, "Push: frame pushed");
				std::shared_ptr<AVFrame> audio(CloneFrame(sync.Audio));
				std::shared_ptr<AVFrame> video(CloneFrame(sync.Video));
				if (audio)
				{
					audio->pts = audio_samples_pushed_;
					audio_samples_pushed_ += audio->nb_samples;
				}
				video->pts = video_frames_pushed_;
				video_frames_pushed_++;
				if (buffer_.try_emplace(Core::AVSync(audio, video, sync.TimeInfo)) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine(Common::DebugSeverity::warning, "Push: frame dropped");
				output_executor_.begin_invoke([this]
					{
						ProcessFrameFromBuffer();
					});
			}

			void RegisterClockTarget(Core::ClockTarget* target)
			{
				DebugPrintLine(Common::DebugSeverity::debug, "RegisterClockTarget");
				std::lock_guard<std::mutex> lock(frame_clock_mutex_);
				frame_clock_targets_.push_back(target);
			}

			void UnregisterClockTarget(Core::ClockTarget* target)
			{
				DebugPrintLine(Common::DebugSeverity::debug, "UnregisterClockTarget");
				std::lock_guard<std::mutex> lock(frame_clock_mutex_);
				output_executor_.invoke([=] { frame_clock_targets_.erase(std::remove(frame_clock_targets_.begin(), frame_clock_targets_.end(), target), frame_clock_targets_.end()); });
			}

		};

		FFmpegOutput::FFmpegOutput(const FFOutputParams params)
			: impl_(std::make_unique<implementation>(params))
		{ }

		FFmpegOutput::~FFmpegOutput() { }

		void FFmpegOutput::Initialize(Core::VideoFormatType video_format, TVPlayR::PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) { impl_->Initialize(video_format, pixel_format, audio_channel_count, audio_sample_rate); }

		void FFmpegOutput::AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) 	{ impl_->AddOverlay(overlay); }

		void FFmpegOutput::RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->RemoveOverlay(overlay); }

		void FFmpegOutput::Push(Core::AVSync& sync) { impl_->Push(sync); }

		void FFmpegOutput::RegisterClockTarget(Core::ClockTarget& target) { impl_->RegisterClockTarget(&target); }

		void FFmpegOutput::UnregisterClockTarget(Core::ClockTarget& target) { impl_->UnregisterClockTarget(&target); }

		const FFOutputParams& FFmpegOutput::GetStreamOutputParams() { return params_; }
	}
}

