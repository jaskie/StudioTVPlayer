#include "../pch.h"
#include "Preview.h"
#include "../Common/Exceptions.h"
#include "../Common/Semaphore.h"
#include "../Common/Executor.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../FFMpeg/VideoFilter.h"


namespace TVPlayR {
	namespace Preview {

	struct Preview::implementation
	{
		FRAME_PLAYED_CALLBACK frame_played_callback_;
		Core::VideoFormat format_;
		std::mutex filter_creation_mutex_;
		std::unique_ptr<FFmpeg::VideoFilter> output_filter_;
		Common::Semaphore frame_ready_semaphore_;
		std::shared_ptr<AVFrame> buffer_frame_;
		std::thread consumer_thread_;
		std::atomic_bool shutdown_thread_;

		implementation()
			: consumer_thread_(&implementation::ConsumerThreadProc, this)
		{
		}

		~implementation()
		{
			shutdown_thread_ = true;
			frame_ready_semaphore_.notify();
			consumer_thread_.join();
		}

		void ConsumerThreadProc()
		{
			Common::SetThreadName(::GetCurrentThreadId(), "Preview thread");
			while (!shutdown_thread_)
			{
				frame_ready_semaphore_.wait();
				if (buffer_frame_)
				{
					std::lock_guard<std::mutex> lock(filter_creation_mutex_);
					if (!output_filter_)
						continue;
				}
			}
		}

		void Push(FFmpeg::AVSync& sync)
		{
			buffer_frame_ = sync.Video;
			frame_ready_semaphore_.notify();
		}

		bool AssignToChannel(Core::Channel& channel)
		{
			format_ = channel.Format();
			return true;
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

	void Preview::SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }

	void Preview::CreateFilter(int width, int height)
	{

	}
}}
