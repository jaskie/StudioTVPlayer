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
			// Inherited via OutputDevice
			virtual bool AssignToChannel(const Core::Channel& channel) override;
			virtual void ReleaseChannel() override;
			virtual void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			virtual void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
			virtual void Push(FFmpeg::AVSync& sync) override;
			virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
			// FFmpegOutput
			const FFOutputParams& GetStreamOutputParams();
		private:
			const FFOutputParams params_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;
			std::vector<std::shared_ptr<Core::OverlayBase>> overlays_;
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};

}}
