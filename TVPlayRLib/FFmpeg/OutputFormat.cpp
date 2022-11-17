#include "../pch.h"
#include "OutputFormat.h"
#include "FFmpegUtils.h"

namespace TVPlayR {
	namespace FFmpeg {
		OutputFormat::OutputFormat(const std::string& url, AVDictionary*& options)
			: Common::DebugTarget(Common::DebugSeverity::info, "OutputFormat " + url)
			, url_(url)
			, options_(options)
			, format_ctx_(AllocFormatContextAndOpenFile(url), [this](AVFormatContext* ctx) { FreeFormatContext(ctx); })
		{
		}

		void OutputFormat::Push(const std::shared_ptr<AVPacket>& packet)
		{
			if (!is_initialized_)
			{
				initialization_queue_.emplace_back(packet);
				DebugPrintLine(Common::DebugSeverity::debug, "Queuing packet to stream=" + std::to_string(packet->stream_index) + ", pts=" + std::to_string(packet->pts) + ", dts=" + std::to_string(packet->dts));
				return;
			}
			DebugPrintLine(Common::DebugSeverity::trace, "Sending packet to stream=" + std::to_string(packet->stream_index) + ", pts=" + std::to_string(packet->pts) + ", dts=" + std::to_string(packet->dts));
			THROW_ON_FFMPEG_ERROR(av_interleaved_write_frame(format_ctx_.get(), packet.get()));
		}

		void OutputFormat::Flush()
		{
			if (!is_initialized_)
				return;
			DebugPrintLine(Common::DebugSeverity::debug, "Flushing");
			THROW_ON_FFMPEG_ERROR(av_interleaved_write_frame(format_ctx_.get(), NULL));
			is_flushed_ = true;
		}

		void OutputFormat::Initialize(const std::string& stream_metadata)
		{
			assert(!is_initialized_);
			DebugPrintLine(Common::DebugSeverity::debug, "Writing header");
			format_ctx_->metadata = ReadOptions(stream_metadata);
			format_ctx_->max_delay = AV_TIME_BASE * 7 / 10;
			format_ctx_->flags = AVFMT_FLAG_FLUSH_PACKETS | format_ctx_->flags;
			THROW_ON_FFMPEG_ERROR(avformat_write_header(format_ctx_.get(), &options_));
			if (DebugSeverity() <= Common::DebugSeverity::info)
				av_dump_format(format_ctx_.get(), 0, url_.c_str(), true);
			while (!initialization_queue_.empty())
			{
				std::shared_ptr<AVPacket> packet = initialization_queue_.front();
				initialization_queue_.pop_front();
				DebugPrintLine(Common::DebugSeverity::trace, "Sending queued packet to stream=" + std::to_string(packet->stream_index) + ", pts=" + std::to_string(packet->pts) + ", dts=" + std::to_string(packet->dts));
				THROW_ON_FFMPEG_ERROR(av_interleaved_write_frame(format_ctx_.get(), packet.get()));
			}
			is_initialized_ = true;
		}

		AVFormatContext* OutputFormat::AllocFormatContextAndOpenFile(const std::string& url)
		{
			const AVOutputFormat* format = nullptr;
			if (url.find("rtmp://") == 0)
				format = av_guess_format("flv", NULL, NULL);
			else if (url.find("udp://") == 0)
				format = av_guess_format("mpegts", NULL, NULL);
			else
				format = av_guess_format(NULL, url.c_str(), NULL);
			if (!format)
				THROW_EXCEPTION("OutputFormat: can't determine format for " + url);

			AVFormatContext* ctx = nullptr;
			THROW_ON_FFMPEG_ERROR(avformat_alloc_output_context2(&ctx, format, NULL, url.c_str()));
			if (!ctx)
				THROW_EXCEPTION("OutputFormat: context for " + url + " not created");
			if (!(ctx->oformat->flags & AVFMT_NOFILE))
				THROW_ON_FFMPEG_ERROR(avio_open2(&ctx->pb, url.c_str(), AVIO_FLAG_WRITE, NULL, &options_));
			return ctx;
		}

		void OutputFormat::FreeFormatContext(AVFormatContext* ctx)
		{
			if (is_initialized_ && !FF(av_write_trailer(ctx)))
				DebugPrintLine(Common::DebugSeverity::warning, "av_write_trailer failed");
			if (!(ctx->oformat->flags & AVFMT_NOFILE))
				if (!FF(avio_close(ctx->pb)))
					DebugPrintLine(Common::DebugSeverity::warning, "avio_close failed");
			avformat_free_context(ctx);
			DebugPrintLine(Common::DebugSeverity::info, "avformat_free_context");
		}

		
	}
}