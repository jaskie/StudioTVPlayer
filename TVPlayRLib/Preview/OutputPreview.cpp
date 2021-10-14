#include "../pch.h"
#include "OutputPreview.h"
#include "../Common/Exceptions.h"
#include "../Common/Executor.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../FFmpeg/SwScale.h"
#include "../PixelFormat.h"


namespace TVPlayR {
	namespace Preview {

	struct OutputPreview::implementation
	{
		FRAME_PLAYED_CALLBACK frame_played_callback_ =nullptr;
		const Core::Channel* channel_;
		std::unique_ptr<FFmpeg::SwScale> preview_scaler_;

		Common::Executor consumer_executor_;
		const int width_, height_;

		implementation(int width, int height)
			: consumer_executor_("OutputPreview thread", 1)
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
				if (!frame_played_callback_)
					return;
				std::shared_ptr<AVFrame> received_frame = preview_scaler_->Scale(frame);
				frame_played_callback_(received_frame);
			});
		}

		bool AssignToChannel(const Core::Channel& channel)
		{
			consumer_executor_.invoke([&] {
				channel_ = &channel;
				preview_scaler_ = std::make_unique<FFmpeg::SwScale>(channel.Format().width(), channel.Format().height(), PixelFormatToFFmpegFormat(channel.PixelFormat()), width_, height_, AV_PIX_FMT_BGRA);
			});
			return true;
		}

		void ReleaseChannel()
		{
			consumer_executor_.invoke([this] {
				channel_ = nullptr;
				preview_scaler_.reset();
			});
		}

		void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback)
		{
			consumer_executor_.invoke([this, frame_played_callback] {
				frame_played_callback_ = frame_played_callback;
			});
		}

	};

	OutputPreview::OutputPreview(int width, int height)
		: impl_(std::make_unique<implementation>(width, height))
	{ }

	OutputPreview::~OutputPreview()	{}

	bool OutputPreview::AssignToChannel(const Core::Channel& channel) { return impl_->AssignToChannel(channel); }

	void OutputPreview::ReleaseChannel() { impl_->ReleaseChannel(); }

	void OutputPreview::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }

	void OutputPreview::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
	{
		THROW_EXCEPTION("The preview cannot act as clock source");
	}

	void OutputPreview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback) { impl_->SetFramePlayedCallback(frame_played_callback); }
}}
