#include "../pch.h"
#include "Preview.h"
#include "../Common/Exceptions.h"
#include "../Common/Executor.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "PreviewScaler.h"


namespace TVPlayR {
	namespace Preview {

	struct Preview::implementation
	{
		FRAME_PLAYED_CALLBACK frame_played_callback_ =nullptr;
		Core::Channel* channel_;
		std::unique_ptr<PreviewScaler> preview_scaler_;
		Common::Executor consumer_executor_;
		const int width_, height_;

		implementation(int width, int height)
			: consumer_executor_("Preview thread")
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
				ProcessFrame(frame);
			});
		}

		void ProcessFrame(std::shared_ptr<AVFrame> frame)
		{
			preview_scaler_->Push(frame);
			std::shared_ptr<AVFrame> received_frame;
			while (received_frame = preview_scaler_->Pull())
			{
				if (!frame_played_callback_)
					continue;
				frame_played_callback_(received_frame);
			}
		}

		bool AssignToChannel(Core::Channel& channel)
		{
			channel_ = &channel;
			preview_scaler_ = std::make_unique<PreviewScaler>(*channel_, width_, height_);
			return true;
		}

		void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			frame_played_callback_ = frame_played_callback;
		}

	};

	Preview::Preview(int width, int height)
		: impl_(std::make_unique<implementation>(width, height))
	{ }

	Preview::~Preview()	{}

	bool Preview::AssignToChannel(Core::Channel& channel) { return impl_->AssignToChannel(channel); }

	void Preview::ReleaseChannel() {}

	void Preview::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }

	void Preview::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
	{
		THROW_EXCEPTION("The preview cannot act as clock source");
	}

	void Preview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback) { impl_->SetFramePlayedCallback(frame_played_callback); }
}}
