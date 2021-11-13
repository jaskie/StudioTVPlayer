#include "../pch.h"
#include "NdiOutput.h"
#include "NdiUtils.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "../Common/BlockingCollection.h"

namespace TVPlayR {
	namespace Ndi {
		
		struct NdiOutput::implementation : Common::DebugTarget
		{
			const std::string source_name_;
			NDIlib_v4* const ndi_;
			const NDIlib_send_instance_t send_instance_;
			Core::VideoFormat format_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			std::int64_t video_frames_pushed_ = 0LL;
			std::int64_t audio_samples_pushed_ = 0LL;
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
				ReleaseChannel();
				executor_.stop();
				if (send_instance_)
					ndi_->send_destroy(send_instance_);
			}

			bool AssignToChannel(const Core::Channel& channel)
			{
				return executor_.invoke([&] 
				{
					if (format_.type() != Core::VideoFormatType::invalid)
					{
						DebugPrintLine("Already assigned to another channel");
						return false;
					}
					format_ = channel.Format();
					audio_sample_rate_ = channel.AudioSampleRate();
					audio_channels_count_ = channel.AudioChannelsCount();
					video_frames_pushed_ = 0LL;
					audio_samples_pushed_ = 0LL;
					executor_.begin_invoke([this] 
					{ 
						if (frame_requested_callback_)
							InitializeFrameRequester();
						DebugPrintLine("AssignToChannel - calling Tick()");
						Tick();
					}); 
					return true;
				});
			}

			void ReleaseChannel()
			{
				executor_.invoke([this] 
				{ 
					format_ = Core::VideoFormatType::invalid;
				});
			}

			void Push(FFmpeg::AVSync& sync)
			{
				if (buffer_.try_add(sync) == Common::BlockingCollectionStatus::Ok)
				{
					video_frames_pushed_++;
					audio_samples_pushed_ += sync.Audio->nb_samples;
				}
				else
					DebugPrintLine("Frame dropped");
			}
						
			void Tick()
			{
				assert(executor_.is_current());
				if (format_.type() == Core::VideoFormatType::invalid)
					return;
				FFmpeg::AVSync buffer;
				if (buffer_.try_take(buffer) == Common::BlockingCollectionStatus::Ok)
				{
					NDIlib_video_frame_v2_t ndi_video = CreateVideoFrame(format_, buffer.Video, buffer.Timecode);
					ndi_->send_send_video_v2(send_instance_, &ndi_video);
					NDIlib_audio_frame_interleaved_32s_t ndi_audio = CreateAudioFrame(buffer.Audio, buffer.Timecode);
					ndi_->util_send_send_audio_interleaved_32s(send_instance_, &ndi_audio);
					RequestFrameFromChannel();
				}
				if (format_.type() != Core::VideoFormatType::invalid)
					executor_.begin_invoke([this] { Tick(); }); // next frame
			}

			int AudioSamplesRequired() 
			{
				assert(executor_.is_current());
				std::int64_t samples_required = av_rescale(video_frames_pushed_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_pushed_;
				return static_cast<int>(samples_required);
			}

			void RequestFrameFromChannel()
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
					RequestFrameFromChannel();
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { 
					frame_requested_callback_ = frame_requested_callback;
					if (frame_requested_callback && format_.type() != Core::VideoFormatType::invalid)
					{
						InitializeFrameRequester();
						DebugPrintLine("SetFrameRequestedCallback - calling Tick()");
						Tick();
					}
				});
			}

		};
			
		NdiOutput::NdiOutput(const std::string& source_name, const std::string& group_name) : impl_(std::make_unique<implementation>(source_name, group_name)) { }
		NdiOutput::~NdiOutput() { }

		bool NdiOutput::AssignToChannel(const Core::Channel& channel) { return impl_->AssignToChannel(channel); }

		void NdiOutput::ReleaseChannel() { impl_->ReleaseChannel(); }

		void NdiOutput::Push(FFmpeg::AVSync & sync) { impl_->Push(sync); }
		
		void NdiOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
		{
			impl_->SetFrameRequestedCallback(frame_requested_callback);
		}
	}
}

