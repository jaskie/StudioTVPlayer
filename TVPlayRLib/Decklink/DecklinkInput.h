#pragma once
#include "../Core/InputSource.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
		enum class VideoFormatType;
		enum class FieldOrder;
	}
	namespace Decklink {
		class DecklinkInput : public Core::InputSource
		{
		public:
			explicit DecklinkInput(IDeckLink* decklink, Core::VideoFormatType format, int audio_channels_count);
			~DecklinkInput();
			virtual FFmpeg::AVSync PullSync(const Core::Channel& channel, int audio_samples_count) override;
			virtual bool IsAddedToChannel(const Core::Channel& channel) override;
			virtual void AddToChannel(const Core::Channel& channel) override;
			virtual void RemoveFromChannel(const Core::Channel& channel) override;
			virtual void Play() override;
			virtual void Pause() override;
			virtual bool IsPlaying() const override;
			virtual int GetWidth() const override;
			virtual int GetHeight() const override;
			virtual Core::FieldOrder GetFieldOrder() override;
			virtual int GetAudioChannelCount() override;
			virtual bool HaveAlphaChannel() const override;
			virtual void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}