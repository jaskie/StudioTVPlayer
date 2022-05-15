#pragma once

namespace TVPlayR {
	namespace FFmpeg {
		struct AVSync
		{
			AVSync(std::shared_ptr<AVFrame> audio, std::shared_ptr<AVFrame> video, std::int64_t timecode, std::int64_t time_from_begin, std::int64_t time_to_end)
				: Audio(audio)
				, Video(video)
				, Timecode(timecode)
				, TimeFromBegin(time_from_begin)
				, TimeToEnd(time_to_end)
			{ }
			AVSync() : AVSync(nullptr, nullptr, AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE) {}
			AVSync(AVSync&& other) = default;
			AVSync(const AVSync& other) = default;
			std::shared_ptr<AVFrame> Audio;
			std::shared_ptr<AVFrame> Video;
			std::int64_t Timecode;
			std::int64_t TimeFromBegin;
			std::int64_t TimeToEnd;
			AVSync operator=(AVSync&& other) noexcept
			{
				if (&other == this)
					return *this;
				Audio = std::move(other.Audio);
				Video = std::move(other.Video);
				Timecode = other.Timecode;
				TimeFromBegin = other.TimeFromBegin;
				TimeToEnd = other.TimeToEnd;
				return *this;
			}
			
			AVSync operator=(const AVSync& other)
			{
				Audio = other.Audio;
				Video = other.Video;
				Timecode = other.Timecode;
				TimeFromBegin = other.TimeFromBegin;
				TimeToEnd = other.TimeToEnd;
				return *this;
			}

			operator bool() const noexcept
			{
				return Audio || Video;
			}
		};


}}
