#pragma once
#include "../Common/NonCopyable.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
		class OutputFormat final : Common::NonCopyable, Common::DebugTarget
		{
		public:
			typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> AVFormatContextPtr;
			OutputFormat(const std::string& url, AVDictionary*& options);
			void Push(std::shared_ptr<AVPacket> packet);
			void Initialize(const std::string& stream_metadata);
			AVFormatContext* Ctx() const { return format_ctx_.get(); }
			const std::string& GetUrl() const { return url_; }
		private:
			const std::string url_;
			AVDictionary*& options_;
			AVFormatContextPtr format_ctx_;
			AVFormatContext* AllocFormatContext(const std::string& url);
			void FreeFormatContext(AVFormatContext* ctx);
		};

	}
}

