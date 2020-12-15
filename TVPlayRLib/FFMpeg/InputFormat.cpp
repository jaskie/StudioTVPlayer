#include "../pch.h"
#include "InputFormat.h"
#include "Utils.h"

namespace TVPlayR {
	namespace FFmpeg {

InputFormat::InputFormat(const std::string& fileName)
{
	AVFormatContext* weak_format_context = NULL;
	THROW_ON_FFMPEG_ERROR(avformat_open_input(&weak_format_context, fileName.c_str(), NULL, NULL) == 0 && weak_format_context);
	if (!weak_format_context)
		THROW_EXCEPTION("Format context not created");
	if (!weak_format_context)
		return;
	format_context_ = AVFormatCtxPtr(weak_format_context, [](AVFormatContext * ctx)
	{
		avformat_close_input(&ctx);
	});
	if (FF(avformat_find_stream_info(weak_format_context, NULL)))
	{
		video_stream_index_ = av_find_best_stream(weak_format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec_, 0);
		auto best_audio_stream_index = av_find_best_stream(weak_format_context, AVMEDIA_TYPE_AUDIO, -1, video_stream_index_, &audio_codec_, 0);
		if (best_audio_stream_index >= 0)
		{
			for (unsigned int i = 0; i < weak_format_context->nb_streams; i++)
				if (weak_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
					&& weak_format_context->streams[i]->codecpar->codec_id == weak_format_context->streams[best_audio_stream_index]->codecpar->codec_id
					&& weak_format_context->streams[i]->codecpar->sample_rate == weak_format_context->streams[best_audio_stream_index]->codecpar->sample_rate)
					audio_streams_.push_back(weak_format_context->streams[i]);
		}
	}
	is_eof_ = false;
}


int64_t InputFormat::GetVideoDuration() const
{
	if (video_stream_index_ == AVERROR_STREAM_NOT_FOUND)
		return 0L;
	return av_rescale_q(format_context_->streams[video_stream_index_]->duration, format_context_->streams[video_stream_index_]->time_base, av_get_time_base_q());
}

int64_t InputFormat::GetAudioDuration() const
{
	if (audio_streams_.size() == 0)
		return 0L;
	auto stream = audio_streams_.front();
	return av_rescale_q(stream->duration, stream->time_base, av_get_time_base_q());
}

const std::vector<AVStream *>& InputFormat::GetAudioStreams() const
{
	return audio_streams_;
}

AVStream * InputFormat::GetVideoStream() const
{
	if (video_stream_index_ == AVERROR_STREAM_NOT_FOUND || !format_context_)
		return NULL;
	return
		format_context_->streams[video_stream_index_];
}

AVCodec * InputFormat::GetAudioCodec() const
{
	return audio_codec_;
}

AVCodec * InputFormat::GetVideoCodec() const
{
	return video_codec_;
}

AVPacketPtr InputFormat::PullPacket()
{
	if (format_context_)
	{
		auto packet = AllocPacket();
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
	return __nullptr;
}

bool InputFormat::Seek(int64_t time)
{
	AVRational time_base = format_context_->streams[video_stream_index_]->time_base;
	int64_t seek_time = av_rescale_q(time - AV_TIME_BASE, av_get_time_base_q(), time_base) - format_context_->streams[video_stream_index_]->start_time;
	if (FF(av_seek_frame(format_context_.get(), video_stream_index_, seek_time, AVSEEK_FLAG_BACKWARD)))
	{
		is_eof_ = false;
		return true;
	}
	return false;
}

	
}}