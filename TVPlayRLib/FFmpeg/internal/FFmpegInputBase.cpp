#include "../../pch.h"
#include "FFmpegInputBase.h"
#include "../Decoder.h"
#include "../../Core/StreamInfo.h"
#include "../../Core/FieldOrder.h"
#include "../FFmpegUtils.h"


namespace TVPlayR {
	namespace FFmpeg {
		namespace internal {

			FFmpegInputBase::FFmpegInputBase(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
				: file_name_(file_name)
				, input_(file_name)
				, acceleration_(acceleration)
				, hw_device_(hw_device)
				, is_stream_(IsStream(file_name))
				, video_decoder_()
			{
			}

			void FFmpegInputBase::InitializeVideoDecoder()
			{
				if (video_decoder_)
					return;
				auto stream = input_.GetVideoStream();
				if (stream == nullptr)
					return;
				video_decoder_ = std::make_unique<Decoder>(stream->Codec, stream->Stream, stream->StartTime, acceleration_, hw_device_);
			}

			bool FFmpegInputBase::IsStream(const std::string& fileName) const
			{
				auto prefix = fileName.substr(0, 6);
				return prefix == "udp://" || prefix == "rtp://";
			}

			int FFmpegInputBase::StreamCount() const
			{
				return static_cast<int>(input_.GetStreams().size());
			}

			const Core::StreamInfo& FFmpegInputBase::GetStreamInfo(int index) const
			{
				auto& streams = input_.GetStreams();
				assert(index >= 0 && index < streams.size());
				return input_.GetStreams()[index];
			}

			int64_t FFmpegInputBase::GetAudioDuration()
			{
				for (auto& stream : input_.GetStreams())
					if (stream.Type == Core::MediaType::audio)
						return stream.Duration;
				return 0LL;
			}

			int64_t FFmpegInputBase::GetVideoStart() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return 0LL;
				return stream->StartTime;
			}

			int64_t FFmpegInputBase::GetVideoDuration() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return 0LL;
				return stream->Duration;
			}

			AVRational FFmpegInputBase::GetTimeBase() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return { 0, 1 };
				return stream->Stream->time_base;
			}

			AVRational FFmpegInputBase::GetFrameRate() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return { 0, 1 };
				return stream->Stream->r_frame_rate;
			}

			int FFmpegInputBase::GetWidth() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return 0;
				return stream->Stream->codecpar->width;
			}

			int FFmpegInputBase::GetHeight() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return 0;
				return stream->Stream->codecpar->height;
			}

			Core::FieldOrder FFmpegInputBase::GetFieldOrder() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return Core::FieldOrder::unknown;
				return Core::FieldOrderFromAVFieldOrder(stream->Stream->codecpar->field_order);
			}

			bool FFmpegInputBase::HaveAlphaChannel() const
			{
				const Core::StreamInfo* stream = input_.GetVideoStream();
				if (stream == nullptr)
					return false;
				return FFmpeg::HaveAlphaChannel(static_cast<AVPixelFormat>(stream->Stream->codecpar->format));
			}

			int FFmpegInputBase::GetAudioChannelCount() const
			{
				return input_.GetTotalAudioChannelCount();
			}

		}
	}
}
