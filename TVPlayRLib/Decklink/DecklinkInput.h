#pragma once
#include "../Core/InputSource.h"

namespace TVPlayR {

	enum class DecklinkTimecodeSource;
	enum class PixelFormat;

	namespace Core {
		enum class VideoFormatType;
	}

	namespace Decklink {

		typedef void(*FORMAT_CALLBACK)(Core::VideoFormatType new_format);

		class DecklinkInput final : public Core::InputSource
		{
		public:
			explicit DecklinkInput(IDeckLink* decklink, Core::VideoFormatType initial_format, PixelFormat pixel_format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video, bool format_autodetection);
			virtual ~DecklinkInput();
			Core::AVSync PullSync(const Core::Player& player, int audio_samples_count) override;
			bool IsAddedToPlayer(const Core::Player& player) override;
			void AddToPlayer(const Core::Player& player) override;
			void RemoveFromPlayer(const Core::Player& player) override;
			void AddOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) override;
			void RemoveOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) override;
			void Play() override;
			void Pause() override;
			bool IsPlaying() const override;
			bool IsEof() const override;
			int GetWidth() const override;
			int GetHeight() const override;
			TVPlayR::FieldOrder GetFieldOrder() override;
			int GetAudioChannelCount() override;
			bool HaveAlphaChannel() const override;
			void SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) override;
			void SetFormatChangedCallback(FORMAT_CALLBACK format_changed_callback);
		private:
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};
	}
}
