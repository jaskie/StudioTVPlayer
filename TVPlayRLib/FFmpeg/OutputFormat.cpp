#include "../pch.h"
#include "OutputFormat.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
		OutputFormat::OutputFormat(const std::string& file_name)
			: Common::DebugTarget(true, "OutputFormat " + file_name)
			, file_name_(file_name)
			, format_ctx_(AllocFormatContext(file_name), [this](AVFormatContext* ctx) { FreeFormatContext(ctx); })
		{
		}

		void OutputFormat::Push(std::shared_ptr<AVPacket> packet)
		{
			DebugPrintLine("Sending packet pts=" + std::to_string(packet->pts));
			THROW_ON_FFMPEG_ERROR(av_interleaved_write_frame(format_ctx_.get(), packet.get()));
		}

		void OutputFormat::Initialize(AVDictionary** options)
		{
			DebugPrintLine("Writing header");
			//format_ctx_->metadata = read_parameters(output_params_.output_metadata_);
			format_ctx_->max_delay = AV_TIME_BASE * 7 / 10;
			format_ctx_->flags = AVFMT_FLAG_FLUSH_PACKETS | format_ctx_->flags;
			if (!(format_ctx_->oformat->flags & AVFMT_NOFILE))
				THROW_ON_FFMPEG_ERROR(avio_open2(&format_ctx_->pb, file_name_.c_str(), AVIO_FLAG_WRITE, NULL, options));
			THROW_ON_FFMPEG_ERROR(avformat_write_header(format_ctx_.get(), options));
		}

		AVFormatContext* OutputFormat::AllocFormatContext(const std::string& file_name)
		{
			AVOutputFormat* format = nullptr;
			if (file_name.find("rtmp://") == 0)
				format = av_guess_format("flv", NULL, NULL);
			else if (file_name.find("udp://") == 0)
				format = av_guess_format("mpegts", NULL, NULL);
			else
				format = av_guess_format(NULL, file_name.c_str(), NULL);
			if (!format)
				THROW_EXCEPTION("Can't determine oformat");

			AVFormatContext* ctx = nullptr;
			THROW_ON_FFMPEG_ERROR(avformat_alloc_output_context2(&ctx, format, NULL, file_name.c_str()));
			if (!ctx)
				THROW_EXCEPTION("Format context not created")
			return ctx;
		}

		void OutputFormat::FreeFormatContext(AVFormatContext* ctx)
		{
			if (!(ctx->oformat->flags & AVFMT_NOFILE))
				if (!FF(avio_close(ctx->pb)))
					DebugPrintLine("avio_close failed");
			avformat_free_context(ctx);
		}

		
	}
}