#pragma once
#include "../Common/NonCopyable.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
		class OutputFormat final : Common::NonCopyable, Common::DebugTarget
		{
		public:
			typedef std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> AVFormatContextPtr;
			OutputFormat(const std::string& file_name, AVDictionary*& options);
			void Push(std::shared_ptr<AVPacket> packet);
			void Initialize(const std::string& stream_metadata);
			AVFormatContext* Ctx() const { return format_ctx_.get(); }
			const std::string& GetFileName() const { return file_name_; }
		private:
			const std::string file_name_;
			AVDictionary*& options_;
			AVFormatContextPtr format_ctx_;
			AVFormatContext* AllocFormatContext(const std::string& file_name);
			void FreeFormatContext(AVFormatContext* ctx);
		};

	}
}

