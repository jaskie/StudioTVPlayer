#include "../pch.h"
#include "InputFormat.h"
#include "FFmpegUtils.h"
#include "../Core/HwAccel.h"
#include "../Core/StreamInfo.h"

namespace TVPlayR {
	namespace FFmpeg {

		AVFormatContext* CreateContext(const std::string& file_name, bool dump)
		{
			AVFormatContext* ctx = NULL;
			THROW_ON_FFMPEG_ERROR(avformat_open_input(&ctx, file_name.c_str(), NULL, NULL) == 0 && ctx);
			if (!ctx)
				THROW_EXCEPTION("InputFormat: context not created for " + file_name);
			if (dump)
				av_dump_format(ctx, 0, file_name.c_str(), 0);
			return ctx;
		}


InputFormat::InputFormat(const std::string& file_name)
	: DebugTarget(Common::DebugSeverity::info, "Input format: " + file_name)
	, format_context_(CreateContext(file_name, DebugSeverity() <= Common::DebugSeverity::info), [](AVFormatContext* ctx){ avformat_close_input(&ctx); })
	, file_name_(file_name)
{
}

std::int64_t InputFormat::ReadStartTimecode() const
{
	for (const Core::StreamInfo& stream : streams_)
	{
		if (stream.Stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
			continue;
		AVDictionaryEntry* tcr = av_dict_get(stream.Stream->metadata, "timecode", NULL, 0);
		if (tcr)
		{
			AVTimecode tc;
			if (FF_SUCCESS(av_timecode_init_from_string(&tc, stream.Stream->r_frame_rate, tcr->value, NULL)))
				return av_rescale((std::int64_t)tc.start * AV_TIME_BASE, tc.rate.den, tc.rate.num);
		}
	}
	return 0LL;
}

bool InputFormat::LoadStreamData()
{
	if (!FF_SUCCESS(avformat_find_stream_info(format_context_.get(), NULL)))
		return false;
	streams_.clear();
	int best_video = av_find_best_stream(format_context_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	for (size_t i = 0; i < format_context_->nb_streams; i++)
	{
		AVStream* stream = format_context_->streams[i];
		if (!stream)
			continue;
		AVDictionaryEntry* language = av_dict_get(stream->metadata, "language", NULL, 0);
		streams_.push_back(Core::StreamInfo{
			stream->index,
			stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ? Core::MediaType::audio : stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ? Core::MediaType::video : Core::MediaType::other,
			stream->index == best_video,
			PtsToTime(stream->start_time, stream->time_base),
			PtsToTime(stream->duration, stream->time_base),
			stream->codecpar->ch_layout.nb_channels,
			language ? language->value : "",
			avcodec_find_decoder(stream->codecpar->codec_id),
			stream
			});
	}
	is_stream_data_loaded_ = true;
	return true;
}

std::shared_ptr<AVPacket> InputFormat::PullPacket()
{
	if (format_context_)
	{
		auto packet = AllocPacket();
		std::lock_guard<std::mutex> lock(seek_mutex_);
		int ret = av_read_frame(format_context_.get(), packet.get());
		switch (ret)
		{
		case AVERROR_EOF:
			is_eof_ = true;
			break;
		case 0:
			return packet;
		default:
			break;
		}
	}
	return nullptr;
}

bool InputFormat::CanSeek() const
{
	if (format_context_->ctx_flags & AVFMTCTX_UNSEEKABLE)
		return false;
	// hack to correctly determine seekability in case of .jpg files
	auto stream = GetVideoStream();
	if (!stream)
		return true;
	return stream->Duration > AV_TIME_BASE/10 || stream->Stream->nb_frames > 1 ;
}

bool InputFormat::Seek(std::int64_t time)
{
	std::lock_guard<std::mutex> lock(seek_mutex_);
	if (!CanSeek())
		return false;
	if (FF_SUCCESS(av_seek_frame(format_context_.get(), -1, time, AVSEEK_FLAG_BACKWARD)))
	{
		is_eof_ = false;
		return true;
	}
	return false;
}

int InputFormat::GetTotalAudioChannelCount() const
{
	int result = 0;
	for (size_t i = 0; i < format_context_->nb_streams; i++)
		if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			result += format_context_->streams[i]->codecpar->ch_layout.nb_channels;
	return result;
}

const Core::StreamInfo* InputFormat::GetVideoStream() const
{
	auto info_iter = std::find_if(streams_.begin(), streams_.end(), [](const Core::StreamInfo& info) { return info.Type == Core::MediaType::video && info.IsPreffered; });
	if (info_iter == streams_.end())
		info_iter = std::find_if(streams_.begin(), streams_.end(), [](const Core::StreamInfo& info) { return info.Type == Core::MediaType::video; });
	if (info_iter == streams_.end())
		return nullptr;
	return &*info_iter;
}

bool InputFormat::IsValid() const
{
	return !!format_context_;
}



	
}}