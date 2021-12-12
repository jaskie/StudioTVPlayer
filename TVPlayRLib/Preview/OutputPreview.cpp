#include "../pch.h"
#include "OutputPreview.h"
#include "../Common/Exceptions.h"
#include "../Core/VideoFormat.h"
#include "../Core/Player.h"
#include "../FFmpeg/SwScale.h"
#include "../PixelFormat.h"
#include "../FFmpeg/AVSync.h"

namespace TVPlayR {
	namespace Preview {

	struct OutputPreview::implementation
	{
		FRAME_PLAYED_CALLBACK frame_played_callback_ =nullptr;
		const Core::Player* player_;
		std::unique_ptr<FFmpeg::SwScale> preview_scaler_;
		const int width_, height_;
		Common::Executor consumer_executor_;

		implementation(int width, int height)
			: consumer_executor_("OutputPreview thread", 1)
			, player_(nullptr)
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

		bool AssignToPlayer(const Core::Player& player)
		{
			consumer_executor_.invoke([&] {
				player_ = &player;
				preview_scaler_ = std::make_unique<FFmpeg::SwScale>(player.Format().width(), player.Format().height(), PixelFormatToFFmpegFormat(player.PixelFormat()), width_, height_, AV_PIX_FMT_BGRA);
			});
			return true;
		}

		void ReleasePlayer()
		{
			consumer_executor_.invoke([this] {
				player_ = nullptr;
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

	bool OutputPreview::AssignToPlayer(const Core::Player& player) { return impl_->AssignToPlayer(player); }

	void OutputPreview::ReleasePlayer() { impl_->ReleasePlayer(); }

	void OutputPreview::AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
	{
		THROW_EXCEPTION("Preview don't support overlays");
	}

	void OutputPreview::RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay)
	{
		THROW_EXCEPTION("Preview don't support overlays");
	}

	void OutputPreview::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }

	void OutputPreview::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
	{
		THROW_EXCEPTION("Preview cannot act as clock source");
	}

	void OutputPreview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback) { impl_->SetFramePlayedCallback(frame_played_callback); }
}}
