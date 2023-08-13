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
			, producer_(&FFmpegBufferedInput::ProducerTheradStart, this)
			, output_audio_parameters_(output_audio_parameters)
		{
			input_.LoadStreamData();
			InitializeBuffer();
			initialization_cv_.notify_one();
		}

		FFmpegBufferedInput::~FFmpegBufferedInput()
		{
			producer_cv_.notify_one();
			producer_.join();
		}

#pragma region Input thread methods

		void FFmpegBufferedInput::ProducerTheradStart()
		{
			Common::SetThreadName(::GetCurrentThreadId(), ("FFmpegBufferedInput " + file_name_).c_str());
			std::unique_lock<std::mutex> init_lock(initialization_mutex_);
			initialization_cv_.wait(init_lock);
			while (is_producer_running_)
			{
				while (!buffer_->IsFull())
					ProcessNextInputPacket();
				{
					std::unique_lock<std::mutex> wait_lock(producer_mutex_);
					producer_cv_.wait(wait_lock);
				}
			}
		}

		void FFmpegBufferedInput::InitializeBuffer()
		{
			InitializeVideoDecoder();
			InitializeAudioDecoders();
			if (!audio_decoders_.empty())
				audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, output_audio_parameters_);
			buffer_ = std::make_unique<SynchronizingBuffer>(
				video_decoder_->FrameRate(),
				output_audio_parameters_,
				AV_TIME_BASE, // 1s
				0,
				input_.ReadStartTimecode(),
				GetVideoDuration(),
				GetFieldOrder()
			);
		}

		void FFmpegBufferedInput::InitializeAudioDecoders()
		{
			auto& streams = input_.GetStreams();
			const Core::StreamInfo *video_stream = input_.GetVideoStream();
			std::int64_t seek = video_stream ? video_stream->StartTime : 0;
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
				buffer_->PushVideo(decoded, video_decoder_->TimeBase());
		}

		void FFmpegBufferedInput::ProcessAudio(const std::unique_ptr<Decoder>& decoder)
		{
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
					std::int64_t seek_time = input_.GetVideoStream()->StartTime;
					input_.Seek(seek_time);
					video_decoder_->Seek(seek_time);
					for (const auto& decoder : audio_decoders_)
						decoder->Seek(seek_time);
					if (audio_muxer_)
						audio_muxer_->Reset();
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

		Core::AVSync FFmpegBufferedInput::PullSync(int audio_samples_count)
		{
			bool finished = false;
			Core::AVSync sync;
			{
				if (is_eof_)
					return buffer_->PullSync(audio_samples_count);
				sync = buffer_->PullSync(audio_samples_count);
				finished = buffer_->IsEof();
			}
			if (finished)
			{
				is_eof_ = true;
			}
			else
				producer_cv_.notify_one();
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
					audio_muxer_->Reset();
				if (buffer_)
					buffer_->Seek(time);
				is_eof_ = false;
				producer_cv_.notify_one();
				return true;
			}
			return false;
		}

    }
}