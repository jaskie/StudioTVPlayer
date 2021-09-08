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
		
		struct NdiOutput::implementation: Common::DebugTarget<false>
		{
			const std::string source_name_;
			volatile bool is_running_ = false;
			NDIlib_v4* const ndi_;
			const NDIlib_send_instance_t send_instance_;
			Core::VideoFormat format_;
			int audio_channels_count_ = 2;
			int audio_sample_rate_ = 48000;
			AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			std::shared_ptr<AVFrame> last_video_;
			Common::BlockingCollection<FFmpeg::AVSync> buffer_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			Common::Executor executor_;
			int64_t video_frames_pushed_ = 0LL;
			int64_t audio_samples_pushed_ = 0LL;
			int64_t last_video_time_ = 0LL;

			implementation(const std::string& source_name, const std::string& group_names)
				: executor_("NDI output " + source_name)
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

			bool AssignToChannel(Core::Channel& channel)
			{
				return executor_.invoke([&] 
				{
					if (is_running_)
						return false;
					is_running_ = true;
					format_ = channel.Format();
					audio_sample_rate_ = channel.AudioSampleRate();
					last_video_ = FFmpeg::CreateEmptyVideoFrame(format_, channel.PixelFormat());
					video_frames_pushed_ = 0LL;
					audio_samples_pushed_ = 0LL;
					last_video_time_ = 0LL;
					executor_.begin_invoke([this] { Tick(); }); // first frame
					return true;
				});
			}

			void ReleaseChannel()
			{
				executor_.invoke([this] 
				{ 
					is_running_ = false;
					format_ = Core::VideoFormatType::invalid;
				});
			}

			void Push(FFmpeg::AVSync& sync)
			{
				if (buffer_.try_add(sync) != Common::BlockingCollectionStatus::Ok)
					DebugPrintLine("NdiOutput " + source_name_ + ": Frame dropped when pushed");
			}
						
			void Tick()
			{
				std::shared_ptr<AVFrame> audio;
				auto buffer = GetBuffer();
				if (buffer.Video)
				{
					last_video_ = buffer.Video;
					last_video_time_ = buffer.Time;
					audio = buffer.Audio;
				}
				else
				{
					audio = FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_, audio_sample_format_);
					DebugPrintLine("NdiOutput " + source_name_ + ": Frame late");
				}
				video_frames_pushed_++;
				audio_samples_pushed_ += audio->nb_samples;
				if (frame_requested_callback_)
					frame_requested_callback_(AudioSamplesRequired());
				NDIlib_video_frame_v2_t ndi_video = CreateVideoFrame(format_, last_video_, last_video_time_);
				ndi_->send_send_video_v2(send_instance_, &ndi_video);
				NDIlib_audio_frame_interleaved_32s_t ndi_audio = CreateAudioFrame(audio, last_video_time_);
				ndi_->util_send_send_audio_interleaved_32s(send_instance_, &ndi_audio);
				if (is_running_)
					executor_.begin_invoke([this] { Tick(); }); // next frame
			}

			FFmpeg::AVSync GetBuffer()
			{
				FFmpeg::AVSync sync;
				buffer_.try_take(sync);
				return sync;
			}

			int AudioSamplesRequired() 
			{
				int64_t samples_required = av_rescale(video_frames_pushed_ + 1LL, audio_sample_rate_ * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - audio_samples_pushed_;
				return static_cast<int>(samples_required);
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				executor_.invoke([&] { frame_requested_callback_ = frame_requested_callback; });
			}

		};
			
		NdiOutput::NdiOutput(const std::string& source_name, const std::string& group_name) : impl_(std::make_unique<implementation>(source_name, group_name)) { }
		NdiOutput::~NdiOutput() { }

		bool NdiOutput::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }

		void NdiOutput::ReleaseChannel() { impl_->ReleaseChannel(); }

		void NdiOutput::Push(FFmpeg::AVSync & sync) { impl_->Push(sync); }
		
		void NdiOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
		{
			impl_->SetFrameRequestedCallback(frame_requested_callback);
		}
	}
}

