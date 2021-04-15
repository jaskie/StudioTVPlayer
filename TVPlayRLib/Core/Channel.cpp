#include "../pch.h"
#include "Channel.h"
#include "InputSource.h"
#include "OutputDevice.h"
#include "AudioVolume.h"
#include "../Common/Executor.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace Core {

		struct Channel::implementation : Common::DebugTarget<false>
		{
			std::vector<std::shared_ptr<OutputDevice>> output_devices_;
			std::shared_ptr<OutputDevice> frame_clock_;
			std::shared_ptr<InputSource> playing_source_;
			std::mutex mutex_;
			AudioVolume audio_volume_;
			const VideoFormat format_;
			const Core::PixelFormat pixel_format_;
			const int audio_channels_count_;
			const std::shared_ptr<AVFrame> empty_video_;
			const AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			AUDIO_VOLUME_CALLBACK audio_volume_callback_ = nullptr;
			Common::Executor frame_requester_;
			

			implementation(const VideoFormatType& format, const Core::PixelFormat pixel_format, const int audio_channels_count)
				: format_(format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, frame_clock_(nullptr)
				, playing_source_(nullptr)
				, empty_video_(FFmpeg::CreateEmptyVideoFrame(format, pixel_format))
				, frame_requester_("Frame requester")
			{
			}

			~implementation()
			{
				if (frame_clock_)
				{
					frame_clock_->SetFrameRequestedCallback(nullptr);
					frame_clock_.reset();
				}
				for (auto device : output_devices_)
					device->ReleaseChannel();
			}

			void RequestFrame(int audio_samples_count)
			{
				frame_requester_.begin_invoke([this, audio_samples_count]
				{
					std::lock_guard<std::mutex> lock(mutex_);
					std::vector<double> volume;
					if (playing_source_)
					{
#ifdef DEBUG
						DebugPrint(("Requested frame with " + std::to_string(audio_samples_count) + " samples of audio\n").c_str());
#endif // DEBUG
						auto sync = playing_source_->PullSync(audio_samples_count);
						assert(sync.Audio->nb_samples == audio_samples_count);
						if (!sync.Video)
							sync.Video = empty_video_;
						volume = audio_volume_.ProcessVolume(sync.Audio);
						for (auto device : output_devices_)
							device->Push(sync);
					
					}
					else
					{
						auto sync = FFmpeg::AVSync(FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channels_count_, audio_sample_format_), empty_video_, 0LL);
						assert(sync.Audio->nb_samples == audio_samples_count);
						for (auto device : output_devices_)
							device->Push(sync);
					}
					if (audio_volume_callback_)
						audio_volume_callback_(volume);
				});
			}

			void AddOutput(std::shared_ptr<OutputDevice>& device)
			{
				output_devices_.push_back(device);
			}

			void RemoveOutput(std::shared_ptr<OutputDevice>& device)
			{
				output_devices_.erase(std::remove(output_devices_.begin(), output_devices_.end(), device), output_devices_.end());
			}

			void SetFrameClock(std::shared_ptr<OutputDevice>& clock)
			{
				if (frame_clock_)
					frame_clock_->SetFrameRequestedCallback(nullptr);
				frame_clock_ = clock;
				clock->SetFrameRequestedCallback(std::bind(&implementation::RequestFrame, this, std::placeholders::_1));
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

		Channel::Channel(const VideoFormatType& format, const Core::PixelFormat pixel_format, const int audio_channels_count)
			: impl_(std::make_unique<implementation>(format, pixel_format, audio_channels_count)) {}
		
		Channel::~Channel() {}

		bool Channel::AddOutput(std::shared_ptr<OutputDevice> device) {
			if (!device->AssignToChannel(*this))
				return false;
			impl_->AddOutput(device);
			return true;
		}

		void Channel::RemoveOutput(std::shared_ptr<OutputDevice> device)
		{
			device->ReleaseChannel();
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

		void Channel::Clear() { impl_->Clear(); }

		const VideoFormat & Channel::Format() const	{ return impl_->format_; }

		const PixelFormat Channel::PixelFormat() const { return impl_->pixel_format_;	}

		const int Channel::AudioChannelsCount() const { return impl_->audio_channels_count_; }

		void Channel::SetVolume(double volume) { impl_->audio_volume_.SetVolume(volume); }

		void Channel::SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback) { impl_->audio_volume_callback_ = callback; }

		void Channel::RequestFrame(int audio_samples_count) { impl_->RequestFrame(audio_samples_count); }

}}