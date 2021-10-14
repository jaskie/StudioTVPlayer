#include "../pch.h"
#include "../PixelFormat.h"
#include "Channel.h"
#include "InputSource.h"
#include "OutputDevice.h"
#include "AudioVolume.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"
#include "OverlayBase.h"

namespace TVPlayR {
	namespace Core {

		struct Channel::implementation : Common::DebugTarget
		{
			const Channel& channel_;
			const std::string name_;
			std::vector<std::shared_ptr<OutputDevice>> output_devices_;
			std::shared_ptr<OutputDevice> frame_clock_;
			std::shared_ptr<InputSource> playing_source_;
			AudioVolume audio_volume_;
			const VideoFormat format_;
			const TVPlayR::PixelFormat pixel_format_;
			const int audio_channels_count_;
			const std::shared_ptr<AVFrame> empty_video_;
			std::vector<std::shared_ptr<OverlayBase>> overlays_;
			const AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			AUDIO_VOLUME_CALLBACK audio_volume_callback_ = nullptr;
			Common::Executor executor_;
		
			implementation(const Channel& channel, const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count)
				: Common::DebugTarget(false, "Channel " + name)
				, channel_(channel)
				, name_(name)
				, format_(format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, frame_clock_(nullptr)
				, playing_source_(nullptr)
				, empty_video_(FFmpeg::CreateEmptyVideoFrame(format, pixel_format))
				, executor_("Channel: " + name)
			{
			}

			~implementation()
			{
				executor_.invoke([this]()
				{
					if (audio_volume_callback_)
						audio_volume_callback_ = nullptr;
					if (frame_clock_)
						frame_clock_->SetFrameRequestedCallback(nullptr);
					frame_clock_ = nullptr;
				});
			}

			void RequestFrame(int audio_samples_count)
			{
				executor_.begin_invoke([this, audio_samples_count]() mutable
				{
					if (audio_samples_count < 0)
						audio_samples_count = 0;
					std::vector<double> volume;
					if (playing_source_)
					{
						DebugPrintLine(("Requested frame with " + std::to_string(audio_samples_count) + " samples of audio").c_str());
						auto sync = playing_source_->PullSync(channel_, audio_samples_count);
						assert(sync.Audio->nb_samples == audio_samples_count);
						if (!sync.Video)
							sync.Video = empty_video_;
						volume = audio_volume_.ProcessVolume(sync.Audio);
						AddOverlayAndPushToOutputs(sync);					
					}
					else
					{
						auto sync = FFmpeg::AVSync(FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channels_count_, audio_sample_format_), empty_video_, 0LL);
						assert(sync.Audio->nb_samples == audio_samples_count);
						AddOverlayAndPushToOutputs(sync);
					}
					if (audio_volume_callback_)
						audio_volume_callback_(volume);
				});
			}

			// used only in executor thread
			void AddOverlayAndPushToOutputs(FFmpeg::AVSync sync)
			{
				for (auto& overlay : overlays_)
					sync = overlay->Transform(sync);
				for (auto& device : output_devices_)
					device->Push(sync);
			}

			void AddOutput(std::shared_ptr<OutputDevice>& device)
			{
				assert(device);
				executor_.invoke([this, &device]
				{
					output_devices_.push_back(device);
				});
			}

			void RemoveOutput(std::shared_ptr<OutputDevice>& device)
			{
				assert(device);
				executor_.invoke([this, &device]
				{
					output_devices_.erase(std::remove(output_devices_.begin(), output_devices_.end(), device), output_devices_.end());
					device->ReleaseChannel();
				});
			}

			void SetFrameClock(std::shared_ptr<OutputDevice>& clock)
			{
				assert(clock);
				executor_.invoke([this, &clock]
				{
					if (frame_clock_)
						frame_clock_->SetFrameRequestedCallback(nullptr);
					frame_clock_ = clock;
					if (clock)
						clock->SetFrameRequestedCallback(std::bind(&implementation::RequestFrame, this, std::placeholders::_1));
				});
			}

			void SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback)
			{
				executor_.invoke([this, &callback]
				{
					audio_volume_callback_ = callback;
				});
			}
			
			void Load(std::shared_ptr<InputSource>& source)
			{
				executor_.invoke([this, &source]
				{
					playing_source_ = source;
				});
			}

			void Clear()
			{
				executor_.invoke([this]
				{
					if (!playing_source_)
						return;
					playing_source_->RemoveFromChannel(channel_);
					playing_source_ = nullptr;
				});
			}

			void AddOverlay(std::shared_ptr<OverlayBase> overlay)
			{
				executor_.invoke([&]
				{
					overlays_.push_back(overlay);
				});
			}

		};

		Channel::Channel(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count)
			: impl_(std::make_unique<implementation>(*this, name, format, pixel_format, audio_channels_count)) {}
		
		Channel::~Channel() {}

		bool Channel::AddOutput(std::shared_ptr<OutputDevice> device) {
			if (!device->AssignToChannel(*this))
				return false;
			impl_->AddOutput(device);
			return true;
		}

		void Channel::RemoveOutput(std::shared_ptr<OutputDevice> device)
		{
			impl_->RemoveOutput(device);
		}

		void Channel::SetFrameClock(std::shared_ptr<OutputDevice> clock) { impl_->SetFrameClock(clock); }

		void Channel::Load(std::shared_ptr<InputSource> source) 
		{
			if (!source->IsAddedToChannel(*this))
				source->AddToChannel(*this);
			impl_->Load(source);
		}

		void Channel::Preload(std::shared_ptr<InputSource> source)
		{
			if (!source->IsAddedToChannel(*this))
				source->AddToChannel(*this);
		}

		void Channel::AddOverlay(std::shared_ptr<OverlayBase> overlay) { impl_->AddOverlay(overlay); }

		void Channel::Clear() { impl_->Clear(); }

		const VideoFormat & Channel::Format() const	{ return impl_->format_; }

		const TVPlayR::PixelFormat Channel::PixelFormat() const { return impl_->pixel_format_;	}

		const int Channel::AudioChannelsCount() const { return impl_->audio_channels_count_; }

		void Channel::SetVolume(double volume) { impl_->audio_volume_.SetVolume(volume); }

		void Channel::SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback) { impl_->SetAudioVolumeCallback(callback); }

		const std::string& Channel::Name() const { return impl_->name_; }

		void Channel::RequestFrame(int audio_samples_count) { impl_->RequestFrame(audio_samples_count); }

}}