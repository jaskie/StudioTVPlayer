#include "../pch.h"
#include "Channel.h"
#include "InputSource.h"
#include "OutputDevice.h"

namespace TVPlayR {
	namespace Core {

		struct Channel::implementation
		{
			std::vector<OutputDevice*> output_devices_;
			OutputDevice* frame_clock_ ;
			std::shared_ptr<InputSource> playing_source_;
			std::mutex mutex_;

			const VideoFormat format_;
			const Core::PixelFormat pixel_format_;
			const int audio_channels_count_;
			const std::shared_ptr<AVFrame> empty_video_;

			implementation(const VideoFormat::Type& format, const Core::PixelFormat pixel_format, const int audio_channels_count)
				: format_(format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, frame_clock_(nullptr)
				, playing_source_(nullptr)
				, empty_video_(FFmpeg::CreateEmptyVideoFrame(format, pixel_format))
			{
			}

			~implementation()
			{
				for (auto device : output_devices_)
					device->ReleaseChannel();
			}

			void RequestFrame(int audio_samples_count)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if (playing_source_)
				{
					auto sync = playing_source_->PullSync(audio_samples_count);
					for (auto device : output_devices_)
						device->Push(sync);
				}
				else
				{
					auto sync = FFmpeg::AVSync(FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channels_count_), empty_video_, 0LL);
					for (auto device : output_devices_)
						device->Push(sync);
				}
			}

			void AddOutput(OutputDevice& device)
			{
				output_devices_.push_back(&device);
			}

			void RemoveOutput(OutputDevice& device)
			{
				output_devices_.erase(std::remove(output_devices_.begin(), output_devices_.end(), &device), output_devices_.end());
			}

			void SetFrameClock(OutputDevice& clock, Channel* channel)
			{
				if (frame_clock_)
					frame_clock_->SetFrameRequestedCallback(nullptr);
				frame_clock_ = &clock;
				clock.SetFrameRequestedCallback(std::bind(&implementation::RequestFrame, this, std::placeholders::_1));
			}

			void Load(std::shared_ptr<InputSource>& source)
			{
				std::lock_guard<std::mutex> guard(mutex_);
				playing_source_ = source;
			}

			void Clear()
			{
				std::lock_guard<std::mutex> guard(mutex_);
				if (!playing_source_)
					return;
				playing_source_->RemoveFromChannel();
				playing_source_ = nullptr;
			}
		};

		Channel::Channel(const VideoFormat::Type& format, const Core::PixelFormat pixel_format, const int audio_channels_count)
			: impl_(new implementation(format, pixel_format, audio_channels_count)) {}
		
		Channel::~Channel() {}

		bool Channel::AddOutput(OutputDevice& device) {
			if (!device.AssignToChannel(*this))
				return false;
			impl_->AddOutput(device);
			return true;
		}

		void Channel::RemoveOutput(OutputDevice& device)
		{
			device.ReleaseChannel();
			impl_->RemoveOutput(device);
		}

		void Channel::SetFrameClock(OutputDevice& clock) {
			impl_->SetFrameClock(clock, this);
		}

		void Channel::Load(std::shared_ptr<InputSource>& source) {
			if (!source->IsAddedToChannel(*this))
				source->AddToChannel(*this);
			impl_->Load(source);
		}
		void Channel::Clear() {
			impl_->Clear();
		}

		const VideoFormat & Channel::Format() const
		{
			return impl_->format_;
		}

		const PixelFormat & Channel::PixelFormat() const
		{
			return impl_->pixel_format_;
		}

		const int Channel::AudioChannelsCount() const
		{
			return impl_->audio_channels_count_;
		}

		void Channel::RequestFrame(int audio_samples_count) {
			impl_->RequestFrame(audio_samples_count);
		}

}}