#pragma once
#include "FFOutputParams.h"
#include "../Core/OutputDevice.h"

namespace TVPlayR {

	namespace FFmpeg {
		class FFmpegOutput final : public Core::OutputDevice
		{
		public:
			FFmpegOutput(const FFOutputParams params);
			~FFmpegOutput();
			// OutputDevice
			bool AssignToPlayer(const Core::Player& player) override;
			void ReleasePlayer() override;
			void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			//OutputSink
			void Push(FFmpeg::AVSync& sync) override;
			//FrameClockSource
			virtual void RegisterClockTarget(Core::ClockTarget* target) override;
			// FFmpegOutput
			const FFOutputParams& GetStreamOutputParams();
		private:
			const FFOutputParams params_;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};

}}
