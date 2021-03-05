#pragma once
#include "Utils.h"

namespace TVPlayR {
	namespace FFmpeg {
		class AVSync
		{
		public:
			AVSync(std::shared_ptr<AVFrame> audio, std::shared_ptr<AVFrame> video, int64_t time)
				: Audio(audio)
				, Video(video)
				, Time(time)
			{ }
			AVSync() : AVSync(nullptr, nullptr, 0LL) {}
			AVSync(AVSync&& other) = default;
			AVSync(const AVSync& other) = default;
			std::shared_ptr<AVFrame> Audio;
			std::shared_ptr<AVFrame> Video;
			int64_t Time;
			AVSync operator=(AVSync&& other) noexcept
			{
				if (&other == this)
					return *this;
				Audio = std::move(other.Audio);
				Video = std::move(other.Video);
				Time = other.Time;
				return *this;
			}
			operator bool() const noexcept
			{
				return Audio || Video;
			}
		};


}}
