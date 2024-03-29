#include "../pch.h"
#include "NdiOutput.h"
#include "NdiUtils.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Player.h"
#include "../Core/OverlayBase.h"
#include "../Core/AVSync.h"
#include "../FFmpeg/SwScale.h"

namespace TVPlayR {
	namespace Ndi {
		
		using namespace std::chrono;
		struct NdiOutput::implementation : Common::DebugTarget
		{
			const std::string source_name_;
			NDIlib_v4* const ndi_;
			const NDIlib_send_instance_t send_instance_;
			Core::VideoFormat format_;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			std::vector<Core::ClockTarget*> clock_targets_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			std::unique_ptr<FFmpeg::SwScale> frame_converter_;
			Common::BlockingCollection<Core::AVSync> buffer_;
			std::int64_t audio_samples_requested_ = 0LL;
			std::int64_t video_frames_requested_ = 0LL;
			Common::Executor executor_;

			implementation(const std::string& source_name, const std::string& group_names)
				: Common::DebugTarget(Common::DebugSeverity::info, "NDI output " + source_name)
				, executor_("NDI output " + source_name)
				, buffer_(2)
				, format_(Core::VideoFormatType::invalid)
				, source_name_(source_name)
				, ndi_(LoadNdi())
				, send_instance_(ndi_ ? CreateSend(ndi_, source_name, group_names) : nullptr)
			{
				if (!ndi_)
					THROW_EXCEPTION("Unable to create NDI output: NDI library not found");
			}

			~implementation()
			{
				DebugPrintLine(Common::DebugSeverity::debug, "Destroying");
				executor_.invoke([this]
					{
						format_ = Core::VideoFormatType::invalid;
						if (send_instance_)
							ndi_->send_destroy(send_instance_);
					});
			}

			void Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate)
			{
				bool success = executor_.invoke([&] 
				{
					if (format_.type() != Core::VideoFormatType::invalid)
					{
						DebugPrintLine(Common::DebugSeverity::warning, "Already assigned to another source");
						return false;
					}
					format_ = video_format;
					audio_sample_rate_ = audio_sample_rate;
					audio_channels_count_ = audio_channel_count;
					audio_samples_requested_ = 0LL;
					video_frames_requested_ = 0LL;
					RequestNextFrame();
					Tick();
					return true;
				});
				if (!success)
					THROW_EXCEPTION("NdiOutput: unable to initalize")
			}

			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				executor_.invoke([=]
					{
						overlays_.emplace_back(overlay);
					});
			}

			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
			{
				executor_.invoke([=]
					{
						overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
					});
			}

			void Push(Core::AVSync& sync)
			{
				if (buffer_.try_add(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine(Common::DebugSeverity::debug, "Frame dropped");
			}
						
			void Tick()
			{
				assert(executor_.is_current());
				if (format_.type() != Core::VideoFormatType::invalid)
				{
					Core::AVSync buffer;
					if (buffer_.try_take(buffer) == Common::BlockingCollectionStatus::Ok)
					{
						if (!(buffer.Video->format == AV_PIX_FMT_BGRA || buffer.Video->format == AV_PIX_FMT_UYVY422))
						{
							if (!frame_converter_)
								frame_converter_ = std::make_unique<FFmpeg::SwScale>(buffer.Video->width, buffer.Video->height, static_cast<AVPixelFormat>(buffer.Video->format), format_.width(), format_.height(), overlays_.empty() ? AV_PIX_FMT_UYVY422 : AV_PIX_FMT_BGRA);
							buffer.Video = frame_converter_->Scale(buffer.Video);
						}
						for (auto& overlay : overlays_)
							buffer = overlay->Transform(buffer);
						NDIlib_video_frame_v2_t ndi_video = Ndi::CreateVideoFrame(format_, buffer.Video, buffer.TimeInfo.Timecode);
						ndi_->send_send_video_v2(send_instance_, &ndi_video);
						if (buffer.Audio)
						{
							auto ndi_audio = Ndi::CreateAudioFrame(buffer.Audio, buffer.TimeInfo.Timecode);
							ndi_->util_send_send_audio_interleaved_32f(send_instance_, &ndi_audio);
						}
					}
					else
						std::this_thread::sleep_for(20ms);
					RequestNextFrame();
					executor_.begin_invoke([this] { Tick(); }); // next frame
				}
			}

			void RequestNextFrame()
			{
				assert(executor_.is_current());
				int audio_samples_required = static_cast<int>(av_rescale(video_frames_requested_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_requested_);
				for (Core::ClockTarget * target : clock_targets_)
					target->RequestFrame(audio_samples_required);
				video_frames_requested_++;
				audio_samples_requested_ += audio_samples_required;
			}

			void RegisterClockTarget(Core::ClockTarget* target)
			{
				executor_.invoke([=] { clock_targets_.push_back(target); });
			}

			void UnregisterClockTarget(Core::ClockTarget* target)
			{
				executor_.invoke([=] { clock_targets_.erase(std::remove(clock_targets_.begin(), clock_targets_.end(), target), clock_targets_.end()); });
			}


		};
			
		NdiOutput::NdiOutput(const std::string& source_name, const std::string& group_name) : impl_(std::make_unique<implementation>(source_name, group_name)) { }
		NdiOutput::~NdiOutput() { }

		void NdiOutput::Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) { impl_->Initialize(video_format, pixel_format, audio_channel_count, audio_sample_rate); }

		void NdiOutput::AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->AddOverlay(overlay); }

		void NdiOutput::RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->RemoveOverlay(overlay); }

		void NdiOutput::Push(Core::AVSync & sync) { impl_->Push(sync); }

		void NdiOutput::RegisterClockTarget(Core::ClockTarget& target) { impl_->RegisterClockTarget(&target); }

		void NdiOutput::UnregisterClockTarget(Core::ClockTarget& target) { impl_->UnregisterClockTarget(&target); }
		
	}
}

