#include "../pch.h"
#include "NdiOutput.h"
#include "NdiUtils.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace Ndi {
		


		struct NdiOutput::implementation: Common::DebugTarget<false>
		{
			Core::Channel * channel_ = nullptr;
			const std::string source_name_;
			const std::string group_name_;
			const NDIlib_send_instance_t send_instance_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			Common::Executor send_executor_;
			std::atomic_int64_t video_frames_pushed_;
			std::atomic_int64_t audio_samples_pushed_;
			
			implementation(const std::string& source_name, const std::string& group_names)
				: send_instance_(GetNdi() ? CreateSend(source_name, group_names) : nullptr)
				, send_executor_("Send thread for " + source_name, 1)
			{				
			}

			~implementation()
			{
				send_executor_.stop();
				if (send_instance_)
					GetNdi()->send_destroy(send_instance_);
			}

			NDIlib_v4* GetNdi()
			{
				static NDIlib_v4* ndi_lib = LoadNdi();
				return ndi_lib;
			}

			NDIlib_send_instance_t CreateSend(const std::string& source_name, const std::string& group_names)
			{
				NDIlib_send_create_t send_create_description;
				send_create_description.p_ndi_name = source_name.c_str();
				send_create_description.p_groups = group_names.c_str();
				send_create_description.clock_audio = false;
				send_create_description.clock_video = true;
				return GetNdi()->send_create(&send_create_description);
			}

			bool AssignToChannel(Core::Channel& channel)
			{
				if (channel_)
					return false;
				send_executor_.invoke([this, &channel]
				{
					channel_ = &channel;
					video_frames_pushed_ = 0;
					audio_samples_pushed_ = 0;
					if (frame_requested_callback_)
						frame_requested_callback_(AudioSamplesRequired());
				});
				return true;
			}

			void ReleaseChannel()
			{
				if (!channel_)
					return;
				send_executor_.invoke([this] { channel_ = nullptr; });
			}

			void Push(FFmpeg::AVSync& sync)
			{
				send_executor_.begin_invoke([this, sync] {
					video_frames_pushed_++;
					audio_samples_pushed_ += sync.Audio->nb_samples;
					if (frame_requested_callback_)
						frame_requested_callback_(AudioSamplesRequired());
					NDIlib_video_frame_v2_t video = CreateVideoFrame(sync.Video, sync.Time);
					GetNdi()->send_send_video_v2(send_instance_, &video);
					NDIlib_audio_frame_interleaved_32s_t audio = CreateAudioFrame(sync.Audio, sync.Time);
					GetNdi()->util_send_send_audio_interleaved_32s(send_instance_, &audio);
				});
			}

			int AudioSamplesRequired() 
			{
				int64_t samples_required = av_rescale(video_frames_pushed_ + 1LL, channel_->AudioSampleRate() * channel_->Format().FrameRate().Denominator(), channel_->Format().FrameRate().Numerator()) - audio_samples_pushed_;
#ifdef DEBUG
				std::stringstream msg;
				msg << "Requested " << samples_required << " samples";
				DebugPrintLine(msg.str());
#endif
				return static_cast<int>(samples_required);
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{
				send_executor_.invoke(
					[this, frame_requested_callback]
				{
					frame_requested_callback_ = frame_requested_callback;
				});
			}

			NDIlib_video_frame_v2_t CreateVideoFrame(const std::shared_ptr<AVFrame>& avframe, int64_t time)
			{
				assert(avframe);
				NDIlib_FourCC_video_type_e fourcc;
				switch (avframe->format)
				{
				case AV_PIX_FMT_BGRA:
					fourcc = NDIlib_FourCC_type_BGRA;
					break;
				case AV_PIX_FMT_UYVY422:
					fourcc = NDIlib_FourCC_type_UYVY;
					break;
				default:
					THROW_EXCEPTION("Invalid format of video frame");
				}
				assert(channel_);
				const Core::VideoFormat& format = channel_->Format();
				NDIlib_frame_format_type_e frame_format_type;
				switch (format.field_mode())
				{
				case Core::VideoFormat::FieldMode::lower:
				case Core::VideoFormat::FieldMode::upper:
					frame_format_type = NDIlib_frame_format_type_interleaved;
					break;
				default:
					frame_format_type = NDIlib_frame_format_type_progressive;
					break;
				}
				return NDIlib_video_frame_v2_t(
					avframe->width,
					avframe->height,
					fourcc,
					format.FrameRate().Numerator(),
					format.FrameRate().Denominator(),
					static_cast<float>(format.SampleAspectRatio().Numerator() * format.width()) / static_cast<float>(format.SampleAspectRatio().Numerator() * format.height()),
					frame_format_type,
					time * 10,
					avframe->data[0],
					avframe->linesize[0]					
				);
			}

			NDIlib_audio_frame_interleaved_32s_t CreateAudioFrame(std::shared_ptr<AVFrame> avframe, int64_t time)
			{
				assert(avframe->format == AV_SAMPLE_FMT_S32);
				return NDIlib_audio_frame_interleaved_32s_t(
					avframe->sample_rate,
					avframe->channels,
					avframe->nb_samples,
					time*10,
					0,
					reinterpret_cast<int32_t*>(avframe->data[0])
				);
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

