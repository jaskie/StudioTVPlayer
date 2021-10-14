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
			void Initialize(AVDictionary** options);
			AVFormatContext* Ctx() const { return format_ctx_.get(); }
			const std::string& GetFileName() const { return file_name_; }
		private:
			AVFormatContextPtr format_ctx_;
			const std::string file_name_;
			AVFormatContext* AllocFormatContext(const std::string& file_name);
			void FreeFormatContext(AVFormatContext* ctx);

		};

	}
}

