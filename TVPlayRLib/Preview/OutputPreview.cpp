#include "../pch.h"
#include "OutputPreview.h"
#include "../Common/Exceptions.h"
#include "../Common/Executor.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "PreviewScaler.h"


namespace TVPlayR {
	namespace Preview {

	struct OutputPreview::implementation
	{
		FRAME_PLAYED_CALLBACK frame_played_callback_ =nullptr;
		Core::Channel* channel_;
		std::unique_ptr<PreviewScaler> preview_scaler_;
		Common::Executor consumer_executor_;
		const int width_, height_;

		implementation(int width, int height)
			: consumer_executor_("OutputPreview thread")
			, channel_(nullptr)
			, width_(width)
			, height_(height)
		{
		}

		~implementation()
		{
			consumer_executor_.stop();
		}

		void Push(FFmpeg::AVSync& sync)
		{
			auto& frame = sync.Video;
			consumer_executor_.begin_invoke([frame, this] {
				preview_scaler_->Push(frame);
				std::shared_ptr<AVFrame> received_frame;
				while (received_frame = preview_scaler_->Pull())
				{
					if (!frame_played_callback_)
						continue;
					frame_played_callback_(received_frame);
				}
			});
		}

		bool AssignToChannel(Core::Channel& channel)
		{
			consumer_executor_.invoke([&] {
				channel_ = &channel;
				preview_scaler_ = std::make_unique<PreviewScaler>(channel.Format().FrameRate().av(), width_, height_);
			});
			return true;
		}

		void ReleaseChannel()
		{
			consumer_executor_.begin_invoke([this] {
				channel_ = nullptr;
				preview_scaler_.reset();
			});
		}

		void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			frame_played_callback_ = frame_played_callback;
		}

	};

	OutputPreview::OutputPreview(int width, int height)
		: impl_(std::make_unique<implementation>(width, height))
	{ }

	OutputPreview::~OutputPreview()	{}

	bool OutputPreview::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }

	void OutputPreview::ReleaseChannel() { impl_->ReleaseChannel(); }

	void OutputPreview::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }

	void OutputPreview::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
	{
		THROW_EXCEPTION("The preview cannot act as clock source");
	}

	void OutputPreview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback) { impl_->SetFramePlayedCallback(frame_played_callback); }
}}
