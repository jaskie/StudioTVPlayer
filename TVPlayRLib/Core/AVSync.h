#pragma once
#include "FrameTimeInfo.h"

namespace TVPlayR {
	namespace Core {
		class AVSync final
		{
		private:
			std::shared_ptr<const AVFrame> _audio;
			std::shared_ptr<const AVFrame> _video;
		public:
			AVSync(const std::shared_ptr<const AVFrame>& audio, const std::shared_ptr<const AVFrame>& video, FrameTimeInfo time_info)
				: _audio(audio)
				, _video(video)
				, TimeInfo(time_info)
			{ }
			AVSync() : AVSync(nullptr, nullptr, FrameTimeInfo()) {}
			AVSync(AVSync&& other) = default;
			AVSync(const AVSync& other) = default;
			FrameTimeInfo TimeInfo;
			AVSync operator=(AVSync&& other) noexcept
			{
				if (&other == this)
					return *this;
				_audio = std::move(other._audio);
				_video = std::move(other._video);
				TimeInfo = std::move(other.TimeInfo);
				return *this;
			}

			const std::shared_ptr<const AVFrame>& Audio() const { return _audio; }
			const std::shared_ptr<const AVFrame>& Video() const { return _video; }
			
			AVSync operator=(const AVSync& other)
			{
				_audio = other._audio;
				_video = other._video;
				TimeInfo = other.TimeInfo;
				return *this;
			}

			operator bool() const noexcept
			{
				return _audio || _video;
			}
		};


}}
