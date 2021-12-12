#include "../pch.h"
#include "../PixelFormat.h"
#include "VideoFormat.h"
#include "Player.h"
#include "InputSource.h"
#include "OutputDevice.h"
#include "AudioVolume.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "../FFmpeg/AVSync.h"
#include "OverlayBase.h"

namespace TVPlayR {
	namespace Core {

		struct Player::implementation : Common::DebugTarget
		{
			const Player& player_;
			const std::string name_;
			std::mutex devices_mutex_;
			std::vector<std::shared_ptr<OutputDevice>> output_devices_;
			std::mutex frame_clock_mutex_;
			std::shared_ptr<OutputDevice> frame_clock_;
			std::shared_ptr<InputSource> playing_source_;
			AudioVolume audio_volume_;
			const VideoFormat format_;
			const TVPlayR::PixelFormat pixel_format_;
			const int audio_channels_count_;
			const int audio_sample_rate_;
			const std::shared_ptr<AVFrame> empty_video_;
			std::vector<std::shared_ptr<OverlayBase>> overlays_;
			const AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_S32;
			std::mutex audio_volume_callback_mutex_;
			AUDIO_VOLUME_CALLBACK audio_volume_callback_ = nullptr;
			Common::Executor executor_;
		
			implementation(const Player& player, const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate)
				: Common::DebugTarget(false, "Player " + name)
				, player_(player)
				, name_(name)
				, format_(format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, audio_sample_rate_(audio_sample_rate)
				, frame_clock_(nullptr)
				, playing_source_(nullptr)
				, empty_video_(FFmpeg::CreateEmptyVideoFrame(format, pixel_format))
				, executor_("Player: " + name)
			{
			}

			~implementation()
			{
				{
					std::lock_guard<std::mutex> lock(audio_volume_callback_mutex_);
					if (audio_volume_callback_)
						audio_volume_callback_ = nullptr;
				}
				{
					std::lock_guard<std::mutex> lock(frame_clock_mutex_);
					if (frame_clock_)
						frame_clock_->SetFrameRequestedCallback(nullptr);
					frame_clock_ = nullptr;
				}
			}

			void RequestFrame(int audio_samples_count)
			{
				if (audio_samples_count < 0)
					audio_samples_count = 0;
				executor_.begin_invoke([this, audio_samples_count]()
				{
					std::vector<double> volume(player_.AudioChannelsCount(), 0.0);
					if (playing_source_)
					{
						DebugPrintLine(("Requested frame with " + std::to_string(audio_samples_count) + " samples of audio").c_str());
						auto sync = playing_source_->PullSync(player_, audio_samples_count);
						auto& audio = sync.Audio;
						auto& video = sync.Video;
						assert((audio_samples_count == 0 && !sync.Audio) || (sync.Audio->nb_samples == audio_samples_count));
						if (!video)
							video = empty_video_;
						if (audio)
							volume = audio_volume_.ProcessVolume(audio);
						AddOverlayAndPushToOutputs(video, audio, sync.Timecode);					
					}
					else
					{
						auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channels_count_, audio_sample_format_);
						assert(audio_samples_count == 0 || audio->nb_samples == audio_samples_count);
						AddOverlayAndPushToOutputs(empty_video_, audio, 0LL);
					}
					std::lock_guard<std::mutex> lock(audio_volume_callback_mutex_);
					if (audio_volume_callback_)
						audio_volume_callback_(volume);
				});
			}

			// used only in executor thread
			void AddOverlayAndPushToOutputs(std::shared_ptr<AVFrame> video, std::shared_ptr<AVFrame> audio, std::int64_t timecode)
			{
				FFmpeg::AVSync sync(audio, video, timecode);
				for (auto& overlay : overlays_)
					sync = overlay->Transform(sync);
				std::lock_guard<std::mutex> lock(devices_mutex_);
				for (auto& device : output_devices_)
					device->Push(sync);
			}

			void AddOutput(std::shared_ptr<OutputDevice>& device)
			{
				assert(device);
				std::lock_guard<std::mutex> lock(devices_mutex_);
				output_devices_.push_back(device);
			}

			void RemoveOutput(std::shared_ptr<OutputDevice>& device)
			{
				assert(device);
				device->ReleasePlayer();
				std::lock_guard<std::mutex> lock(devices_mutex_);
				output_devices_.erase(std::remove(output_devices_.begin(), output_devices_.end(), device), output_devices_.end());
			}

			void SetFrameClock(std::shared_ptr<OutputDevice>& clock)
			{
				std::lock_guard<std::mutex> lock(frame_clock_mutex_);
				if (frame_clock_)
					frame_clock_->SetFrameRequestedCallback(nullptr);
				frame_clock_ = clock;
				if (clock)
					clock->SetFrameRequestedCallback(std::bind(&implementation::RequestFrame, this, std::placeholders::_1));
			}

			void SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback)
			{
				std::lock_guard<std::mutex> lock(audio_volume_callback_mutex_);
				audio_volume_callback_ = callback;
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
					playing_source_->RemoveFromPlayer(player_);
					playing_source_ = nullptr;
				});
			}

			void AddOverlay(std::shared_ptr<OverlayBase>& overlay)
			{
				executor_.invoke([&]
				{
					overlays_.emplace_back(overlay);
				});
			}

			void RemoveOverlay(std::shared_ptr<OverlayBase>& overlay)
			{
				executor_.invoke([&]
				{
					overlays_.erase(std::remove(overlays_.begin(), overlays_.end(), overlay), overlays_.end());
				});
			}


		};

		Player::Player(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate)
			: impl_(std::make_unique<implementation>(*this, name, format, pixel_format, audio_channels_count, audio_sample_rate)) {}
		
		Player::~Player() {}

		bool Player::AddOutput(std::shared_ptr<OutputDevice> device) {
			if (!device->AssignToPlayer(*this))
				return false;
			impl_->AddOutput(device);
			return true;
		}

		void Player::RemoveOutput(std::shared_ptr<OutputDevice> device)
		{
			impl_->RemoveOutput(device);
		}

		void Player::SetFrameClock(std::shared_ptr<OutputDevice> clock) { impl_->SetFrameClock(clock); }

		void Player::Load(std::shared_ptr<InputSource> source) 
		{
			if (!source->IsAddedToPlayer(*this))
				source->AddToPlayer(*this);
			impl_->Load(source);
		}

		void Player::Preload(std::shared_ptr<InputSource> source)
		{
			if (!source->IsAddedToPlayer(*this))
				source->AddToPlayer(*this);
		}

		void Player::AddOverlay(std::shared_ptr<OverlayBase> overlay) { impl_->AddOverlay(overlay); }

		void Player::RemoveOverlay(std::shared_ptr<OverlayBase> overlay) { impl_->RemoveOverlay(overlay); }

		void Player::Clear() { impl_->Clear(); }

		const VideoFormat & Player::Format() const	{ return impl_->format_; }

		const TVPlayR::PixelFormat Player::PixelFormat() const { return impl_->pixel_format_;	}

		const int Player::AudioChannelsCount() const { return impl_->audio_channels_count_; }

		const int Player::AudioSampleRate() const { return impl_->audio_sample_rate_; }

		void Player::SetVolume(double volume) { impl_->audio_volume_.SetVolume(volume); }

		void Player::SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback) { impl_->SetAudioVolumeCallback(callback); }

		const std::string& Player::Name() const { return impl_->name_; }


}}