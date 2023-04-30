#include "../pch.h"
#include "../PixelFormat.h"
#include "VideoFormat.h"
#include "Player.h"
#include "InputSource.h"
#include "OutputDevice.h"
#include "AudioVolume.h"
#include "../FFmpeg/FFmpegUtils.h"
#include "AVSync.h"
#include "OverlayBase.h"

namespace TVPlayR {
	namespace Core {

		struct Player::implementation : Common::DebugTarget
		{
			const Player& player_;
			const std::string name_;
			std::mutex devices_mutex_;
			std::vector<std::shared_ptr<OutputSink>> outputs_;
			std::shared_ptr<InputSource> playing_source_;
			std::shared_ptr<InputSource> next_to_play_source_;
			AudioVolume audio_volume_;
			const VideoFormat format_;
			const TVPlayR::PixelFormat pixel_format_;
			const int audio_channels_count_;
			const int audio_sample_rate_;
			const std::shared_ptr<AVFrame> empty_video_;
			std::vector<std::shared_ptr<OverlayBase>> overlays_;
			const AVSampleFormat audio_sample_format_ = AVSampleFormat::AV_SAMPLE_FMT_FLT;
			std::mutex audio_volume_callback_mutex_;
			AUDIO_VOLUME_CALLBACK audio_volume_callback_ = nullptr;
			Common::Executor executor_;
		
			implementation(const Player& player, const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate)
				: Common::DebugTarget(Common::DebugSeverity::warning, "Player " + name)
				, player_(player)
				, name_(name)
				, format_(format)
				, pixel_format_(pixel_format)
				, audio_channels_count_(audio_channels_count)
				, audio_sample_rate_(audio_sample_rate)
				, playing_source_(nullptr)
				, next_to_play_source_(nullptr)
				, empty_video_(FFmpeg::CreateEmptyVideoFrame(format, pixel_format))
				, executor_("Player: " + name)
			{
			}

			~implementation()
			{
				DebugPrintLine(Common::DebugSeverity::info, "Destroying");
				std::lock_guard<std::mutex> lock(audio_volume_callback_mutex_);
				if (audio_volume_callback_)
					audio_volume_callback_ = nullptr;
			}

			void RequestFrame(int audio_samples_count)
			{
				if (audio_samples_count < 0)
					audio_samples_count = 0;
				executor_.begin_invoke([this, audio_samples_count]()
				{
					std::vector<float> volume(player_.AudioChannelsCount(), 0.0);
					float coherence = 0.0;
					DebugPrintLine(Common::DebugSeverity::trace, "Requested frame with " + std::to_string(audio_samples_count) + " samples of audio");
					if (playing_source_)
					{
						auto sync = playing_source_->PullSync(player_, audio_samples_count);
						auto& audio = sync.Audio;
						auto& video = sync.Video;
						assert((audio_samples_count == 0 && !sync.Audio) || (sync.Audio->nb_samples == audio_samples_count));
						if (!video) {
							video = empty_video_;
							DebugPrintLine(Common::DebugSeverity::warning, "Played empty video frame");
						}
						if (audio)
							volume = audio_volume_.ProcessVolume(audio, &coherence);
						AddOverlayAndPushToOutputs(video, audio, sync.TimeInfo);
						if (playing_source_->IsEof() && next_to_play_source_)
						{
							playing_source_ = next_to_play_source_;
							next_to_play_source_->RaiseLoaded();
							next_to_play_source_.reset();
						}
					}
					else
					{
						auto audio = FFmpeg::CreateSilentAudioFrame(audio_samples_count, audio_channels_count_, audio_sample_format_);
						assert(audio_samples_count == 0 || audio->nb_samples == audio_samples_count);
						AddOverlayAndPushToOutputs(empty_video_, audio, FrameTimeInfo());
					}
					std::lock_guard<std::mutex> lock(audio_volume_callback_mutex_);
					if (audio_volume_callback_)
						audio_volume_callback_(volume, coherence);
				});
			}

			// used only in executor thread
			void AddOverlayAndPushToOutputs(std::shared_ptr<AVFrame> video, std::shared_ptr<AVFrame> audio, FrameTimeInfo time_info)
			{
				Core::AVSync sync(audio, video, time_info);
				for (auto& overlay : overlays_)
					sync = overlay->Transform(sync);
				std::lock_guard<std::mutex> lock(devices_mutex_);
				for (auto& device : outputs_)
					device->Push(sync);
			}

			void AddOutputSink(std::shared_ptr<OutputSink>& device)
			{
				assert(device);
				std::lock_guard<std::mutex> lock(devices_mutex_);
				outputs_.push_back(device);
			}

			void RemoveOutputSink(std::shared_ptr<OutputSink>& device)
			{
				assert(device);
				std::lock_guard<std::mutex> lock(devices_mutex_);
				outputs_.erase(std::remove(outputs_.begin(), outputs_.end(), device), outputs_.end());
			}

			void SetFrameClockSource(Player& self, FrameClockSource& clock)
			{
				clock.RegisterClockTarget(self);
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
					playing_source_->RaiseLoaded();
				});
			}

			void PlayNext(std::shared_ptr<InputSource>& source)
			{
				executor_.invoke([this, &source]
				{
					next_to_play_source_ = source;
				});
			}

			void Clear()
			{
				executor_.invoke([this]
				{
					if (!playing_source_)
						return;
					playing_source_->RemoveFromPlayer(player_);
					playing_source_.reset();
					next_to_play_source_.reset();
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

			void SetVolume(float volume)
			{
				executor_.begin_invoke([this, volume]
					{
						audio_volume_.SetVolume(volume);
					});
			}


		};

		Player::Player(const std::string& name, const VideoFormatType& format, TVPlayR::PixelFormat pixel_format, int audio_channels_count, int audio_sample_rate)
			: impl_(std::make_unique<implementation>(*this, name, format, pixel_format, audio_channels_count, audio_sample_rate)) {}
		
		Player::~Player() {}

		void Player::AddOutputSink(std::shared_ptr<OutputSink> sink) {
			impl_->AddOutputSink(sink);
		}

		void Player::RemoveOutputSink(std::shared_ptr<OutputSink> sink)
		{
			impl_->RemoveOutputSink(sink);
		}

		void Player::SetFrameClockSource(FrameClockSource& clock) { impl_->SetFrameClockSource(*this, clock); }

		void Player::RequestFrame(int audio_samples_count) { impl_->RequestFrame(audio_samples_count); }

		void Player::Load(std::shared_ptr<InputSource> source) 
		{
			if (!source->IsAddedToPlayer(*this))
				source->AddToPlayer(*this);
			impl_->Load(source);
		}

		void Player::PlayNext(std::shared_ptr<InputSource> source)
		{
			if (!source->IsAddedToPlayer(*this))
				source->AddToPlayer(*this);
			impl_->PlayNext(source);
		}

		void Player::AddOverlay(std::shared_ptr<OverlayBase> overlay) { impl_->AddOverlay(overlay); }

		void Player::RemoveOverlay(std::shared_ptr<OverlayBase> overlay) { impl_->RemoveOverlay(overlay); }

		void Player::Clear() { impl_->Clear(); }

		const VideoFormat & Player::Format() const	{ return impl_->format_; }

		const TVPlayR::PixelFormat Player::PixelFormat() const { return impl_->pixel_format_;	}

		const int Player::AudioChannelsCount() const { return impl_->audio_channels_count_; }

		const AVSampleFormat Player::AudioSampleFormat() const { return impl_->audio_sample_format_; }

		const int Player::AudioSampleRate() const { return impl_->audio_sample_rate_; }

		void Player::SetVolume(float volume) { impl_->SetVolume(volume); }

		void Player::SetAudioVolumeCallback(AUDIO_VOLUME_CALLBACK callback) { impl_->SetAudioVolumeCallback(callback); }

		const std::string& Player::Name() const { return impl_->name_; }


}}