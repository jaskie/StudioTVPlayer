#pragma once

namespace TVPlayR {
	namespace FFmpeg {
		struct AVSync
		{
			AVSync(std::shared_ptr<AVFrame> audio, std::shared_ptr<AVFrame> video, std::int64_t timecode)
				: Audio(audio)
				, Video(video)
				, Timecode(timecode)
			{ }
			AVSync() : AVSync(nullptr, nullptr, 0LL) {}
			AVSync(AVSync&& other) = default;
			AVSync(const AVSync& other) = default;
			std::shared_ptr<AVFrame> Audio;
			std::shared_ptr<AVFrame> Video;
			std::int64_t Timecode;
			AVSync operator=(AVSync&& other) noexcept
			{
				if (&other == this)
					return *this;
				Audio = std::move(other.Audio);
				Video = std::move(other.Video);
				Timecode = other.Timecode;
				return *this;
			}
			
			AVSync operator=(const AVSync& other)
			{
				Audio = other.Audio;
				Video = other.Video;
				Timecode = other.Timecode;
				return *this;
			}

			operator bool() const noexcept
			{
				return Audio || Video;
			}
		};


}}
