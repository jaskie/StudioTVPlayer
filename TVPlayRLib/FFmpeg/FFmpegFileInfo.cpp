#include "../pch.h"
#include "FFmpegFileInfo.h"
#include "FFmpegUtils.h"
#include "../Common/Semaphore.h"
#include "../Common/Executor.h"
#include "InputFormat.h"
#include "Decoder.h"
#include "../Core/Channel.h"
#include "../Core/FieldOrder.h"
#include "AudioMuxer.h"
#include "SynchronizingBuffer.h"
#include "ChannelScaler.h"
#include "../Core/AudioChannelMapEntry.h"
#include "../Core/StreamInfo.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {

struct FFmpegFileInfo::implementation : Common::DebugTarget<false>
{
	const std::string file_name_;
	const Core::HwAccel acceleration_;
	const std::string hw_device_;
	InputFormat input_;
	std::atomic_bool is_eof_ = false;
	std::atomic_bool is_playing_ = false;
	const bool is_stream_;
	std::unique_ptr<Decoder> video_decoder_;

	implementation(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
		: file_name_(file_name)
		, input_(file_name)
		, acceleration_(acceleration)
		, hw_device_(hw_device)
		, is_stream_(IsStream(file_name))
		, video_decoder_()
	{ 
		input_.LoadStreamData();
		InitializeVideoDecoder();
	}

	~implementation()
	{
	}


	void InitializeVideoDecoder()
	{
		if (video_decoder_)
			return;
		auto stream = input_.GetVideoStream();
		if (stream == nullptr)
			return;
		video_decoder_ = std::make_unique<Decoder>(stream->Codec, stream->Stream, stream->StartTime, acceleration_, hw_device_);
	}

	bool IsStream(const std::string& fileName)
	{
		auto prefix = fileName.substr(0, 6);
		return prefix == "udp://" || prefix == "rtp://";
	}

	int StreamCount() 
	{
		return static_cast<int>(input_.GetStreams().size());
	}

	Core::StreamInfo& GetStreamInfo(int index)
	{
		auto& streams = input_.GetStreams();
		assert(index >= 0 && index < streams.size());
		return input_.GetStreams()[index];
	}

	int64_t GetAudioDuration()
	{
		for (auto& stream : input_.GetStreams())
			if (stream.Type == Core::MediaType::audio)
				return stream.Duration;
		return 0LL;
	}

	int64_t GetVideoStart() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return 0LL;
		return stream->StartTime;
	}

	int64_t GetVideoDuration() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return 0LL;
		return stream->Duration;
	}

	AVRational GetTimeBase() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return { 0, 1 };
		return stream->Stream->time_base;
	}

	AVRational GetFrameRate() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return {0, 1};
		return stream->Stream->r_frame_rate;
	}

	int GetWidth() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return 0;
		return stream->Stream->codecpar->width;
	}

	int GetHeight() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return 0;
		return stream->Stream->codecpar->height;
	}

	Core::FieldOrder GetFieldOrder() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return Core::FieldOrder::unknown;
		return Core::FieldOrderFromAVFieldOrder(stream->Stream->codecpar->field_order);
	}

	bool HaveAlphaChannel() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return false;
		return FFmpeg::HaveAlphaChannel(static_cast<AVPixelFormat>(stream->Stream->codecpar->format));
	}

	int GetAudioChannelCount() const
	{
		return input_.GetTotalAudioChannelCount();
	}

	std::shared_ptr<AVFrame> GetFrameAt(int64_t time)
	{
		input_.Seek(time);
		video_decoder_->Seek(time);
		while (!video_decoder_->IsEof())
		{
			auto packet = input_.PullPacket();
			if (!packet)
				if (input_.IsEof())
					return nullptr;
				else 
					continue;
			if (packet->stream_index != video_decoder_->StreamIndex())
				continue;
			video_decoder_->Push(packet);
			auto frame = video_decoder_->Pull();
			if (frame)
				return frame;
		}
		return nullptr;
	}

};


FFmpegFileInfo::FFmpegFileInfo(const std::string & file_name, Core::HwAccel acceleration, const std::string& hw_device)
	: impl_(std::make_unique<implementation>(file_name, acceleration, hw_device))
{ }

FFmpegFileInfo::~FFmpegFileInfo(){}
std::shared_ptr<AVFrame> FFmpegFileInfo::GetFrameAt(int64_t time)	{ return impl_->GetFrameAt(time); }
int64_t FFmpegFileInfo::GetAudioDuration() const		{ return impl_->GetAudioDuration(); }
int64_t FFmpegFileInfo::GetVideoStart() const		{ return impl_->GetVideoStart(); }
int64_t FFmpegFileInfo::GetVideoDuration() const		{ return impl_->GetVideoDuration(); }
AVRational FFmpeg::FFmpegFileInfo::GetTimeBase() const { return impl_->GetTimeBase(); }
AVRational FFmpeg::FFmpegFileInfo::GetFrameRate() const { return impl_->GetFrameRate(); }
int FFmpeg::FFmpegFileInfo::GetWidth() const { return impl_->GetWidth(); }
int FFmpeg::FFmpegFileInfo::GetHeight() const { return impl_->GetHeight(); }
Core::FieldOrder FFmpeg::FFmpegFileInfo::GetFieldOrder() { return impl_->GetFieldOrder(); }
int FFmpeg::FFmpegFileInfo::GetAudioChannelCount() { return impl_->GetAudioChannelCount(); }
bool FFmpegFileInfo::HaveAlphaChannel() const { return impl_->HaveAlphaChannel(); }
int FFmpegFileInfo::StreamCount() const				{ return impl_->StreamCount(); }
Core::StreamInfo& FFmpegFileInfo::GetStreamInfo(int index) { return impl_->GetStreamInfo(index); }
bool FFmpegFileInfo::IsStream() const { return impl_->is_stream_; }

}}