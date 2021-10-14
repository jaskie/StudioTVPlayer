#include "../pch.h"
#include "FFStreamOutput.h"
#include "../Core/Channel.h"
#include "../PixelFormat.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "OutputFormat.h"
#include "Encoder.h"
#include "SwScale.h"
#include "SwResample.h"

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
			SwScale video_scaler_;
			SwResample audio_resampler_;
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
				, audio_encoder_(output_format_, params.AudioCodec, params.AudioBitrate, channel.AudioSampleRate(), channel.AudioChannelsCount(), &options_, params.AudioMetadata, params.AudioStreamId)
				, video_encoder_(output_format_, params.VideoCodec, params.VideoBitrate, channel.Format(), &options_, params.VideoMetadata, params.VideoStreamId)
				, video_scaler_(format_.width(), format_.height(), PixelFormatToFFmpegFormat(channel.PixelFormat()), video_encoder_.Width(), video_encoder_.Height(), static_cast<AVPixelFormat>(video_encoder_.Format()))
				, audio_resampler_(channel.AudioChannelsCount(), channel.AudioSampleRate(), channel.AudioSampleFormat(), channel.AudioChannelsCount(), audio_encoder_.SampleRate(), static_cast<AVSampleFormat>(audio_encoder_.Format()))
				, buffer_(2)
			{
				output_format_.Initialize(&options_);
#ifdef DEBUG
				av_dump_format(output_format_.Ctx(), 0, params.Address.c_str(), true);
#endif
			}
			
			void Tick() 
			{
				executor_.begin_invoke([this] {
					AVSync sync;
					if (buffer_.try_take(sync) == TVPlayR::Common::BlockingCollectionStatus::Ok)
					{
						video_encoder_.Push(video_scaler_.Scale(sync.Video));
						audio_encoder_.Push(audio_resampler_.Resample(sync.Audio));
						last_video_ = sync.Video;
						PushVideoPackets();
						PushtAudioPackets();
					}
					else
					{

					}
					
				});
			}

			void PushVideoPackets()
			{
				while (true)
				{
					auto packet = video_encoder_.Pull();
					if (!packet)
						return;
					output_format_.Push(packet);
				}
			}

			void PushtAudioPackets()
			{
				while (true)
				{
					auto packet = audio_encoder_.Pull();
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

