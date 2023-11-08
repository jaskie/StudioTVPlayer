#pragma once
#include "FrameTimeInfo.h"

namespace TVPlayR {
	namespace Core {
		struct AVSync
		{
			AVSync(const std::shared_ptr<AVFrame>& audio, const std::shared_ptr<AVFrame>& video, const FrameTimeInfo& time_info)
				: Audio(audio)
				, Video(video)
				, TimeInfo(time_info)
			{ }
			AVSync() 
				: Audio(nullptr)
				, Video(nullptr)
				, TimeInfo{ 0 }
			{}
			AVSync(AVSync&& other) = default;
			AVSync(const AVSync& other) = default;
			std::shared_ptr<AVFrame> Audio;
			std::shared_ptr<AVFrame> Video;
			FrameTimeInfo TimeInfo;
			AVSync operator=(AVSync&& other) noexcept
			{
				if (&other == this)
					return *this;
				Audio = std::move(other.Audio);
				Video = std::move(other.Video);
				TimeInfo = std::move(other.TimeInfo);
				return *this;
			}
			
			AVSync operator=(const AVSync& other)
			{
				Audio = other.Audio;
				Video = other.Video;
				TimeInfo = other.TimeInfo;
				return *this;
			}

			operator bool() const noexcept
			{
				return Audio || Video;
			}
		};


}}
