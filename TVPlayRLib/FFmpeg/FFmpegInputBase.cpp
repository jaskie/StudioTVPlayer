#include "../pch.h"
#include "FFmpegInputBase.h"
#include "Decoder.h"
#include "../Core/StreamInfo.h"
#include "../FieldOrder.h"
#include "FFmpegUtils.h"


namespace TVPlayR {
	namespace FFmpeg {
		bool IsFilenameStream(const std::string& fileName)
		{
			auto prefix = fileName.substr(0, 6);
			return prefix == "udp://" || prefix == "rtp://";
		}

		FFmpegInputBase::FFmpegInputBase(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
			: file_name_(file_name)
			, input_(file_name)
			, acceleration_(acceleration)
			, hw_device_(hw_device)
			, is_stream_(IsFilenameStream(file_name))
			, video_decoder_()
		{
		}

		FFmpegInputBase::~FFmpegInputBase()
		{
		}

		void FFmpegInputBase::InitializeVideoDecoder()
		{
			if (video_decoder_)
				return;
			for (auto& stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
				{
					video_decoder_ = std::make_unique<Decoder>(stream.Codec, stream.Stream, stream.StartTime, acceleration_, hw_device_);
					return;
				}
		}

		bool FFmpegInputBase::IsStream() const
		{
			return is_stream_;
		}

		int FFmpegInputBase::StreamCount() const
		{
			return static_cast<int>(input_.GetStreams().size());
		}

		const Core::StreamInfo& FFmpegInputBase::GetStreamInfo(int index) const
		{
			auto &streams = input_.GetStreams();
			assert(index >= 0 && index < streams.size());
			return streams[index];
		}

		std::int64_t FFmpegInputBase::GetAudioDuration() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::audio)
					return stream.Duration;
			return 0LL;
		}

		std::int64_t FFmpegInputBase::GetVideoStart() const
		{
			return input_.GetVideoStartTime();
		}

		std::int64_t FFmpegInputBase::GetVideoDuration() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return stream.Duration;
			return 0LL;
		}

		AVRational FFmpegInputBase::GetTimeBase() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return stream.Stream->time_base;
			return { 0, 1 };
		}

		AVRational FFmpegInputBase::GetFrameRate() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return stream.Stream->r_frame_rate;
			return { 0, 1 };
		}

		int FFmpegInputBase::GetWidth() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return stream.Stream->codecpar->width;
			return 0;
		}

		int FFmpegInputBase::GetHeight() const
		{
			for (auto& stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return stream.Stream->codecpar->height;
			return 0;
		}

		TVPlayR::FieldOrder FFmpegInputBase::GetFieldOrder() const
		{
			for (auto &stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return TVPlayR::FieldOrderFromAVFieldOrder(stream.Stream->codecpar->field_order);
			return TVPlayR::FieldOrder::Unknown;
		}

		bool FFmpegInputBase::HaveAlphaChannel() const
		{
			for (auto& stream : input_.GetStreams())
				if (stream.Type == Core::MediaType::video)
					return FFmpeg::HaveAlphaChannel(static_cast<AVPixelFormat>(stream.Stream->codecpar->format));
			return false;
		}

		int FFmpegInputBase::GetAudioChannelCount() const
		{
			return input_.GetTotalAudioChannelCount();
		}

	}
}
