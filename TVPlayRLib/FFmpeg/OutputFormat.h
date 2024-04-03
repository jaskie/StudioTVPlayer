#pragma once
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
		class OutputFormat final : private Common::NonCopyable, private Common::DebugTarget
		{
		public:
			OutputFormat(const std::string &url, const std::string &format_name, AVDictionary *&options);
			void Push(const std::shared_ptr<AVPacket> &packet);
			void Flush();
			void Initialize(const std::string& stream_metadata);
			AVFormatContext* Ctx() const { return format_ctx_.get(); }
			const std::string& GetUrl() const { return url_; }
			bool IsFlushed() const { return is_flushed_; }
		private:
			const std::string url_;
			AVDictionary *&options_;
			unique_ptr<AVFormatContext> format_ctx_;
			bool is_initialized_ = false;
			bool is_flushed_ = false;
			std::deque<std::shared_ptr<AVPacket>> initialization_queue_;
			AVFormatContext * AllocFormatContextAndOpenFile(const std::string &url, const std::string &format_name);
			void FreeFormatContext(AVFormatContext* ctx);
		};

	}
}

