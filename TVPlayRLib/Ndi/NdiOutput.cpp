#include "../pch.h"
#include "NdiOutput.h"
#include "NdiUtils.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Player.h"
#include "../Core/OverlayBase.h"
#include "../FFmpeg/AVSync.h"

namespace TVPlayR {
	namespace Ndi {
		
		struct NdiOutput::implementation : Common::DebugTarget
		{
			const std::string source_name_;
			NDIlib_v4* const ndi_;
			const NDIlib_send_instance_t send_instance_;
			Core::VideoFormat format_;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			std::int64_t audio_samples_requested_ = 0LL;
			std::int64_t video_frames_requested_ = 0LL;
			Common::Executor executor_;

			implementation(const std::string& source_name, const std::string& group_names)
				: Common::DebugTarget(false, "NDI output " + source_name)
				, executor_("NDI output " + source_name)
				, buffer_(4)
				, format_(Core::VideoFormatType::invalid)
				, source_name_(source_name)
				, ndi_(LoadNdi())
				, send_instance_(ndi_ ? CreateSend(ndi_, source_name, group_names) : nullptr)
			{				
			}

			~implementation()
			{
				ReleasePlayer();
				executor_.stop();
				if (send_instance_)
					ndi_->send_destroy(send_instance_);
			}

			bool AssignToPlayer(const Core::Player& player)
			{
				return executor_.invoke([&] 
				{
					if (format_.type() != Core::VideoFormatType::invalid)
					{
						DebugPrintLine("Already assigned to another player");
						return false;
					}
					format_ = player.Format();
					audio_sample_rate_ = player.AudioSampleRate();
					audio_channels_count_ = player.AudioChannelsCount();
					if (frame_requested_callback_)
						executor_.begin_invoke([this] 
						{ 
							DebugPrintLine("AssignToPlayer - calling InitializeFrameRequester()");
							InitializeFrameRequester();
						}); 
					return true;
				});
			}

			void ReleasePlayer()
			{
				executor_.invoke([this] 
				{ 
					format_ = Core::VideoFormatType::invalid;
				});
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

			void Push(FFmpeg::AVSync& sync)
			{
				if (buffer_.try_add(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine("Frame dropped");
			}
						
			void Tick()
			{
				assert(executor_.is_current());
				if (format_.type() != Core::VideoFormatType::invalid)
				{
					FFmpeg::AVSync buffer;
					if (buffer_.try_take(buffer) == Common::BlockingCollectionStatus::Ok)
					{
						for (auto& overlay : overlays_)
							buffer = overlay->Transform(buffer);
						NDIlib_video_frame_v2_t ndi_video = CreateVideoFrame(format_, buffer.Video, buffer.Timecode);
						ndi_->send_send_video_v2(send_instance_, &ndi_video);
						if (buffer.Audio)
						{
							auto ndi_audio = CreateAudioFrame(buffer.Audio, buffer.Timecode);
							ndi_->util_send_send_audio_interleaved_32f(send_instance_, &ndi_audio);
						}
						RequestFrameFromPlayer();
					}
				}
				executor_.begin_invoke([this] { Tick(); }); // next frame
			}

			void RequestFrameFromPlayer()
			{
				assert(executor_.is_current());
				if (!frame_requested_callback_)
					return;
				int audio_samples_required = static_cast<int>(av_rescale(video_frames_requested_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_requested_);
				frame_requested_callback_(audio_samples_required);
				video_frames_requested_++;
				audio_samples_requested_ += audio_samples_required;
			}

			void InitializeFrameRequester()
			{
				assert(executor_.is_current());
				audio_samples_requested_ = 0LL;
				video_frames_requested_ = 0LL;
				while (video_frames_requested_ <= static_cast<std::int64_t>(buffer_.bounded_capacity() / 2))
					RequestFrameFromPlayer();
				Tick();
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { 
					frame_requested_callback_ = frame_requested_callback;
					if (frame_requested_callback && format_.type() != Core::VideoFormatType::invalid)
					{
						DebugPrintLine("SetFrameRequestedCallback - calling InitializeFrameRequester()");
						InitializeFrameRequester();
					}
				});
			}

		};
			
		NdiOutput::NdiOutput(const std::string& source_name, const std::string& group_name) : impl_(std::make_unique<implementation>(source_name, group_name)) { }
		NdiOutput::~NdiOutput() { }

		bool NdiOutput::AssignToPlayer(const Core::Player& player) { return impl_->AssignToPlayer(player); }

		void NdiOutput::ReleasePlayer() { impl_->ReleasePlayer(); }

		void NdiOutput::AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->AddOverlay(overlay); }

		void NdiOutput::RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) { impl_->RemoveOverlay(overlay); }

		void NdiOutput::Push(FFmpeg::AVSync & sync) { impl_->Push(sync); }
		
		void NdiOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
		{
			impl_->SetFrameRequestedCallback(frame_requested_callback);
		}
	}
}

