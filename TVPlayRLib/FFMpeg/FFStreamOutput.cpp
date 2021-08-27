#include "../pch.h"
#include "FFStreamOutput.h"
#include "FFStreamOutputParams.h"
#include "../Core/Channel.h"

namespace TVPlayR {
	namespace FFmpeg {

		struct FFStreamOutput::implementation
		{
			const FFStreamOutputParams params_;

			implementation(const FFStreamOutputParams& params)
				:params_(params)
			{

			}
			
			~implementation()
			{
			}

			bool AssignToChannel(Core::Channel& channel)
			{
				return false;
			}

			void ReleaseChannel()
			{

			}

			void Push(FFmpeg::AVSync& sync)
			{
			
			}

			void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
			{

			}

		};

		FFStreamOutput::FFStreamOutput(const FFStreamOutputParams& params)
			: impl_(std::make_unique<implementation>(params))
		{ }

		FFStreamOutput::~FFStreamOutput() { }
		bool FFStreamOutput::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }
		void FFStreamOutput::ReleaseChannel() { impl_->ReleaseChannel(); }
		void FFStreamOutput::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void FFStreamOutput::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->SetFrameRequestedCallback(frame_requested_callback); }

		const FFStreamOutputParams& FFStreamOutput::GetStreamOutputParams() { return impl_->params_; }

	}
}

