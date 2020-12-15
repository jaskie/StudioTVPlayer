#include "../pch.h"
#include "Channel.h"
#include "InputSource.h"
#include "OutputDeviceSource.h"
#include "OutputFrameClock.h"
#include "OutputDevice.h"

namespace TVPlayR {
	namespace Core {

		struct Channel::implementation
		{
			std::vector<std::shared_ptr<OutputDevice>> output_devices_;
			std::shared_ptr<OutputFrameClock> frame_clock_;
			std::shared_ptr<OutputDeviceSource> playing_source_;
			std::mutex mutex_;

			const VideoFormat format_;
			const Core::PixelFormat pixel_format_;
			const int audio_channels_count_;

			implementation(const VideoFormat::Type& Format, const Core::PixelFormat pixel_format, const int audio_channels_count)
				: format_(Format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, frame_clock_(nullptr)
			{
			}

			~implementation()
			{
				for (auto device : output_devices_)
					device->ReleaseChannel();
			}

			void RequestFrame(int audio_samples_count)
			{
				std::lock_guard<std::mutex> guard(mutex_);
				if (!playing_source_)
					return;
				auto video = playing_source_->PullVideo();
				auto audio = playing_source_->PullAudio(audio_samples_count);
				for (auto device : output_devices_)
					device->Push(video, audio);
			}

			void AddOutput(std::shared_ptr<OutputDevice> device)
			{
				output_devices_.push_back(device);
			}

			void SetFrameClock(std::shared_ptr<OutputFrameClock> clock, Channel* channel)
			{
				if (frame_clock_)
				{
					frame_clock_->ReleaseChannel(channel);
				}
				frame_clock_ = clock;
				clock->AssignToChannel(channel);
			}

			void Load(std::shared_ptr<InputSource>& source)
			{
				std::lock_guard<std::mutex> guard(mutex_);
				playing_source_.reset(new OutputDeviceSource(source, format_, pixel_format_, audio_channels_count_));
			}

			void Clear()
			{
				std::lock_guard<std::mutex> guard(mutex_);
				playing_source_.reset();
			}
		};

		Channel::Channel(const VideoFormat::Type& format, const Core::PixelFormat pixel_format, const int audio_channels_count)
			: impl_(new implementation(format, pixel_format, audio_channels_count)) {}
		
		Channel::~Channel() {}

		bool Channel::AddOutput(std::shared_ptr<OutputDevice> device) {
			if (!device->AssignToChannel(this))
				return false;
			impl_->AddOutput(device);
			return true;
		}

		void Channel::SetFrameClock(std::shared_ptr<OutputFrameClock> clock) {
			impl_->SetFrameClock(clock, this);
		}

		void Channel::Load(std::shared_ptr<InputSource>& source) {
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