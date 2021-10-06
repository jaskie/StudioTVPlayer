#pragma once
#include "../Common/NonCopyable.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
		class OutputFormat final : Common::NonCopyable, Common::DebugTarget
		{
		public:
			typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> AVFormatContextPtr;
			OutputFormat(const std::string& file_name);
			void Push(std::shared_ptr<AVPacket> packet);
			const AVFormatContextPtr& Ctx() const { return format_ctx_; }
		private:
			AVFormatContextPtr format_ctx_;
			AVFormatContext* AllocFormatContext(const std::string& file_name);
			void FreeFormatContext(AVFormatContext* ctx);

		};

	}
}

