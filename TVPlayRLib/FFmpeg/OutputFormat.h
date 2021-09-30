#pragma once
#include "../Common/NonCopyable.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
		class OutputFormat final : Common::NonCopyable, Common::DebugTarget
		{
		public:
			OutputFormat(const std::string& file_name);
			void Push(std::shared_ptr<AVPacket> packet);
		private:
			typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> AVFormatContextPtr;
			AVFormatContextPtr format_ctx_;
			AVFormatContext* AllocFormatContext(const std::string& file_name);
			void FreeFormatContext(AVFormatContext* ctx);
		};

	}
}

