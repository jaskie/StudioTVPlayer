#include "../pch.h"
#include "FFmpegBufferedInput.h"
#include "../Core/StreamInfo.h"
#include "../Core/AVSync.h"
#include "SynchronizingBuffer.h"
#include "AudioMuxer.h"
#include "Decoder.h"

namespace TVPlayR {
	namespace FFmpeg {
		FFmpegBufferedInput::FFmpegBufferedInput(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device, const Core::AudioParameters & output_audio_parameters)
			: FFmpegInputBase(file_name, acceleration, hw_device)
			, Common::DebugTarget(Common::DebugSeverity::info, "FFmpegBufferedInput " + file_name)
			, output_audio_parameters_(output_audio_parameters)
		{
			input_.LoadStreamData();
			Initialize();
			producerThread_ = std::thread(&FFmpegBufferedInput::ProducerTheradStart, this);
		}

		FFmpegBufferedInput::~FFmpegBufferedInput()
		{
			producerThread_.join();
		}

#pragma region Input thread methods

		void FFmpegBufferedInput::ProducerTheradStart()
		{
#ifdef DEBUG
			Common::SetThreadName(::GetCurrentThreadId(), ("FFmpegBufferedInput " + file_name_).c_str());
#endif
			while (true)
			{
				ProcessNextInputPacket();
			}
		}

		void FFmpegBufferedInput::Initialize()
		{
			InitializeVideoDecoder();
			InitializeAudioDecoders();
			if (!audio_decoders_.empty())
				audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, output_audio_parameters_);
			std::int64_t media_duration = GetVideoDuration();
			if (!media_duration)
				media_duration = GetAudioDuration();
			buffer_ = std::make_unique<SynchronizingBuffer>(
				video_decoder_->FrameRate(),
				video_decoder_->TimeBase(),
				output_audio_parameters_,
				AV_TIME_BASE, // 1s
				input_.ReadStartTimecode(),
				media_duration
			);
		}

		void FFmpegBufferedInput::InitializeAudioDecoders()
		{
			std::int64_t seek = 0;
			auto& streams = input_.GetStreams();
			for (auto &video_stream : input_.GetStreams())
				if (video_stream.Type == Core::MediaType::video)
				{
					seek = video_stream.StartTime;
					break;
				}
			for (const auto& stream : streams)
			{
				if (stream.Type == Core::MediaType::audio)
					audio_decoders_.emplace_back(std::make_unique<Decoder>(stream.Codec, stream.Stream, seek ? seek : stream.StartTime));
			}
		}

		void FFmpegBufferedInput::ProcessNextInputPacket()
		{
			if (buffer_->IsEof())
				return;
			auto packet = input_.PullPacket();
			if (!packet)
			{
				assert(input_.IsEof());
				for (const auto& decoder : audio_decoders_)
				{
					if (!decoder->IsFlushed())
						decoder->Flush();
					ProcessAudio(decoder);
				}
				FlushAudioMuxerIfNeeded();
				if (video_decoder_)
				{
					if (!video_decoder_->IsFlushed())
						video_decoder_->Flush();
					ProcessVideo();
				}
				FlushBufferOrLoopIfNeeded();
			}
			else	// there is no need to flush if packets are comming
			{
				if (video_decoder_ && packet->stream_index == video_decoder_->StreamIndex())
				{
					video_decoder_->Push(packet);
					ProcessVideo();
				}
				else
					for (const auto& audio_decoder : audio_decoders_)
					{
						if (packet->stream_index == audio_decoder->StreamIndex())
						{
							audio_decoder->Push(packet);
							ProcessAudio(audio_decoder);
							break;
						}
					}
			}
		}

		void FFmpegBufferedInput::ProcessVideo()
		{
			while (auto decoded = video_decoder_->Pull())
				buffer_->PushVideo(decoded);
		}

		void FFmpegBufferedInput::ProcessAudio(const std::unique_ptr<Decoder>& decoder)
		{
			if (decoder->IsEof() || audio_muxer_->IsEof())
				return;
			auto decoded = decoder->Pull();
			if (decoded)
				audio_muxer_->Push(decoder->StreamIndex(), decoded);
			while (auto muxed = audio_muxer_->Pull())
				buffer_->PushAudio(muxed);
		}

		void FFmpegBufferedInput::FlushAudioMuxerIfNeeded()
		{
			if (!audio_muxer_ || audio_muxer_->IsFlushed())
				return;
			auto need_flush = std::all_of(audio_decoders_.begin(), audio_decoders_.end(), [](const std::unique_ptr<Decoder>& decoder) { return decoder->IsEof(); });
			if (need_flush)
				audio_muxer_->Flush();
		}

		void FFmpegBufferedInput::FlushBufferOrLoopIfNeeded()
		{
			if (!buffer_->IsFlushed() && // not flushed yet
				(video_decoder_->IsEof()) && // scaler will provide no more frames
				(!audio_muxer_ || audio_muxer_->IsEof())) // muxer exists and is Eof
				if (is_loop_)
				{
					std::int64_t seek_time = input_.GetVideoStartTime();
					input_.Seek(seek_time);
					video_decoder_->Seek(seek_time);
					for (const auto& decoder : audio_decoders_)
						decoder->Seek(seek_time);
					if (audio_muxer_)
						audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, output_audio_parameters_);
					if (buffer_)
						buffer_->Loop();
					DebugPrintLine(Common::DebugSeverity::info, "Loop");
				}
				else
				{
					buffer_->Flush();
				}
		}
#pragma endregion

		Core::AVSync FFmpegBufferedInput::PullSync()
		{
			bool finished = false;
			Core::AVSync sync;
			{
				if (is_eof_)
					return buffer_->PullSync();
				sync = buffer_->PullSync();
				finished = buffer_->IsEof();
			}
			if (finished)
			{
				is_eof_ = true;
			}
			return sync;
		}

		bool FFmpegBufferedInput::Seek(const std::int64_t time)
		{
			if (input_.Seek(time))
			{
				DebugPrintLine(Common::DebugSeverity::info, "Seek: " + std::to_string(time / 1000));
				if (video_decoder_)
					video_decoder_->Seek(time);
				for (auto& decoder : audio_decoders_)
					decoder->Seek(time);
				if (audio_muxer_)
					audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, output_audio_parameters_);
				if (buffer_)
					buffer_->Seek(time);
				is_eof_ = false;
				return true;
			}
			return false;
		}

    }
}