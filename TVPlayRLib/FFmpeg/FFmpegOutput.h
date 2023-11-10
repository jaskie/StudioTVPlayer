#pragma once
#include "FFOutputParams.h"
#include "../Core/OutputDevice.h"

namespace TVPlayR {

	namespace FFmpeg {
		class FFmpegOutput final : public Core::OutputDevice
		{
		public:
			FFmpegOutput(const FFOutputParams params);
			virtual ~FFmpegOutput();
			// OutputDevice
			void Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) override;
			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			//OutputSink
			void Push(const Core::AVSync& sync) override;
			//FrameClockSource
			void RegisterClockTarget(Core::ClockTarget& target) override;
			void UnregisterClockTarget(Core::ClockTarget& target) override;
			// FFmpegOutput
			const FFOutputParams& GetStreamOutputParams();
		private:
			const FFOutputParams params_;
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};

}}
