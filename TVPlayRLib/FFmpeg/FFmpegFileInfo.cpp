#include "../pch.h"
#include "FFmpegFileInfo.h"
#include "FFmpegInputBase.h"
#include "Decoder.h"
#include "FFmpegUtils.h"
#include "../FieldOrder.h"
#include "../Core/AudioChannelMapEntry.h"
#include "../Core/StreamInfo.h"

namespace TVPlayR {
	namespace FFmpeg {

struct FFmpegFileInfo::implementation : FFmpegInputBase
{

	implementation(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
		: FFmpegInputBase(file_name, acceleration, hw_device)
	{ 
		input_.LoadStreamData();
		InitializeVideoDecoder();
	}

	std::shared_ptr<AVFrame> GetFrameAt(std::int64_t time)
	{
		if (!input_.IsValid())
			return nullptr;
		input_.Seek(time);
		if (!video_decoder_)
			return nullptr;
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

FFmpegFileInfo::~FFmpegFileInfo() {}
std::shared_ptr<AVFrame> FFmpegFileInfo::GetFrameAt(std::int64_t time)		{ return impl_->GetFrameAt(time); }
std::int64_t FFmpegFileInfo::GetAudioDuration() const						{ return impl_->GetAudioDuration(); }
std::int64_t FFmpegFileInfo::GetVideoStart() const							{ return impl_->GetVideoStart(); }
std::int64_t FFmpegFileInfo::GetVideoDuration() const						{ return impl_->GetVideoDuration(); }
AVRational FFmpegFileInfo::GetTimeBase() const							{ return impl_->GetTimeBase(); }
AVRational FFmpegFileInfo::GetFrameRate() const							{ return impl_->GetFrameRate(); }
int FFmpegFileInfo::GetWidth() const									{ return impl_->GetWidth(); }
int FFmpegFileInfo::GetHeight() const									{ return impl_->GetHeight(); }
TVPlayR::FieldOrder FFmpegFileInfo::GetFieldOrder() const				{ return impl_->GetFieldOrder(); }
int FFmpegFileInfo::GetAudioChannelCount() const						{ return impl_->GetAudioChannelCount(); }
bool FFmpegFileInfo::HaveAlphaChannel() const							{ return impl_->HaveAlphaChannel(); }
int FFmpegFileInfo::StreamCount() const									{ return impl_->StreamCount(); }
const Core::StreamInfo& FFmpegFileInfo::GetStreamInfo(int index) const	{ return impl_->GetStreamInfo(index); }
bool FFmpegFileInfo::IsStream() const									{ return impl_->IsStream(); }

}}