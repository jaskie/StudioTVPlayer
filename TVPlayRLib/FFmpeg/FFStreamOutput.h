#pragma once
#include "FFStreamOutputParams.h"
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace FFmpeg {
		class FFStreamOutput final : public Core::OutputDevice
		{
		public:
			FFStreamOutput(const FFStreamOutputParams& params);
			~FFStreamOutput();
			// Inherited via OutputDevice
			virtual bool AssignToChannel(const Core::Channel& channel) override;
			virtual void ReleaseChannel() override;
			virtual void Push(FFmpeg::AVSync& sync) override;
			virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
			// FFStreamOutput
			const FFStreamOutputParams& GetStreamOutputParams();
		private:
			FFStreamOutputParams params_;
			struct implementation;
			std::unique_ptr<implementation> impl_;
		};

}}
