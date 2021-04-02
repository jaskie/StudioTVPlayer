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
		std::mutex mutex_;
		std::unique_ptr<PreviewScaler> preview_scaler_;
		Common::Executor consumer_executor_;

		implementation()
			: consumer_executor_("Preview thread")
			, channel_(nullptr)
		{
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
			std::lock_guard<std::mutex> lock(mutex_);
			if (!preview_scaler_ || !frame_played_callback_)
				return;
			preview_scaler_->Push(frame);
			std::shared_ptr<AVFrame> received_frame;
			while (received_frame = preview_scaler_->Pull())
					frame_played_callback_(received_frame);

		}

		bool AssignToChannel(Core::Channel& channel)
		{
			channel_ = &channel;
			return true;
		}

		void CreateFilter(int width, int height)
		{
			assert(channel_);
			std::lock_guard<std::mutex> lock(mutex_);
			preview_scaler_ = std::make_unique<PreviewScaler>(*channel_, width, height);
		}

		void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			frame_played_callback_ = frame_played_callback;
		}

	};

	Preview::Preview()
		: impl_(std::make_unique<implementation>())
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

	void Preview::CreateFilter(int width, int height) { impl_->CreateFilter(width, height); }
}}
