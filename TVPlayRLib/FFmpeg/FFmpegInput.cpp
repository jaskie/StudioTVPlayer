#include "../pch.h"
#include "FFmpegInput.h"
#include "FFmpegInputBase.h"
#include "FFmpegUtils.h"
#include "../Core/AVSync.h"
#include "Decoder.h"
#include "../Core/Player.h"
#include "AudioMuxer.h"
#include "SynchronizingBuffer.h"
#include "PlayerScaler.h"
#include "../Core/StreamInfo.h"


namespace TVPlayR {
	namespace FFmpeg {
			   		 
struct FFmpegInput::implementation : Common::DebugTarget, FFmpegInputBase
{
	std::atomic_bool is_eof_ = false;
	std::atomic_bool is_playing_ = false;
	std::atomic_bool is_loop_ = false;
	bool is_producer_running_ = false;
	std::vector<std::unique_ptr<Decoder>> audio_decoders_;
	const Core::Player* player_ = nullptr;
	std::unique_ptr<AudioMuxer> audio_muxer_;
	std::unique_ptr<PlayerScaler> player_scaler_;
	std::unique_ptr<SynchronizingBuffer> buffer_;
	std::mutex player_scaler_reset_mutex_;
	std::mutex buffer_content_mutex_;
	std::mutex buffer_mutex_;
	std::condition_variable buffer_cv_;

	TIME_CALLBACK frame_played_callback_ = nullptr;
	PAUSED_CALLBACK paused_callback_ = nullptr;
	std::mutex producer_mutex_;
	std::condition_variable producer_cv_;
	std::thread producer_;

	implementation(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
		: FFmpegInputBase(file_name, acceleration, hw_device)
		, Common::DebugTarget(Common::DebugSeverity::debug, "FFmpegInput " + file_name)
		, producer_(&implementation::ProducerTheradStart, this)
	{ 
		input_.LoadStreamData();
	}

	~implementation()
	{
		if (player_)
			RemoveFromPlayer(*player_);
		{
			is_producer_running_ = false;
			producer_cv_.notify_one();
		}
		producer_.join();
	}

#pragma region Input thread methods

	void ProducerTheradStart()
	{
#ifdef DEBUG
		Common::SetThreadName(("FFmpegInput " + file_name_).c_str());
#endif
		is_producer_running_ = true;
		std::unique_lock<std::mutex> wait_lock(producer_mutex_);
		producer_cv_.wait(wait_lock, [&]()
			{
				std::lock_guard<std::mutex> lock(buffer_mutex_);
				if (player_)
				{
					if (!buffer_)
						InitializeBuffer();
					while (!buffer_->IsFull())
					{
						ProcessNextInputPacket();
						if (buffer_->IsReady())
						{
							std::lock_guard<std::mutex> buffer_content_lock(buffer_content_mutex_);
							buffer_cv_.notify_one();
						}
					}
				}
				return !is_producer_running_;
			});
	}

	void InitializeBuffer()
	{
		InitializeVideoDecoder();
		InitializeAudioDecoders();
		player_scaler_ = std::make_unique<PlayerScaler>(*player_);
		if (!audio_decoders_.empty())
			audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, player_->AudioSampleFormat(), 48000, player_->AudioChannelsCount());
		buffer_ = std::make_unique<SynchronizingBuffer>(
			player_,
			is_playing_,
			AV_TIME_BASE, // 1s
			0,
			input_.ReadStartTimecode(),
			GetVideoDuration(),
			GetFieldOrder()
			);
	}

	void InitializeAudioDecoders()
	{
		auto& streams = input_.GetStreams();
		auto stream = input_.GetVideoStream();
		std::int64_t seek = stream ? stream->StartTime : 0;
		if (std::any_of(streams.begin(), streams.end(), [](const auto& stream) { return stream.Type == Core::MediaType::audio && stream.Language == "pol"; }))
			for (const auto& stream : streams)
			{
				if (stream.Type == Core::MediaType::audio && stream.Language == "pol")
					audio_decoders_.emplace_back(std::make_unique<Decoder>(stream.Codec, stream.Stream, seek ? seek : stream.StartTime));
			}
		else
			for (const auto& stream : streams)
			{
				if (stream.Type == Core::MediaType::audio)
					audio_decoders_.emplace_back(std::make_unique<Decoder>(stream.Codec, stream.Stream, seek ? seek : stream.StartTime));
			}
	}

	void ProcessNextInputPacket()
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
				FlushPlayerScalerIfNeeded();
			}
			FlushBufferOrLoopIfNeeded();
		}
		else 	// there is no need to flush if packets are comming
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

	void ProcessVideo()
	{
		auto decoded = video_decoder_->Pull();
		if (decoded)
		{
			std::lock_guard<std::mutex> lock(player_scaler_reset_mutex_);
			player_scaler_->Push(decoded, video_decoder_->FrameRate(), video_decoder_->TimeBase());
		}
		std::lock_guard<std::mutex> lock(buffer_content_mutex_);
		while (auto scaled = player_scaler_->Pull())
			buffer_->PushVideo(scaled, player_scaler_->OutputTimeBase());
	}

	void ProcessAudio(const std::unique_ptr<Decoder>& decoder)
	{
		auto decoded = decoder->Pull();
		if (decoded)
			audio_muxer_->Push(decoder->StreamIndex(), decoded);
		std::lock_guard<std::mutex> lock(buffer_content_mutex_);
		while (auto muxed = audio_muxer_->Pull())
			buffer_->PushAudio(muxed);
	}

	void FlushPlayerScalerIfNeeded()
	{
		if (video_decoder_->IsEof() && !player_scaler_->IsFlushed())
			player_scaler_->Flush();
	}

	void FlushAudioMuxerIfNeeded()
	{
		if (!audio_muxer_ || audio_muxer_->IsFlushed())
			return;
		auto need_flush = std::all_of(audio_decoders_.begin(), audio_decoders_.end(), [](const std::unique_ptr<Decoder>& decoder) { return decoder->IsEof(); });
		if (need_flush)
			audio_muxer_->Flush();
	}

	void FlushBufferOrLoopIfNeeded()
	{
		if (!buffer_->IsFlushed() && // not flushed yet
			(player_scaler_->IsEof() || (video_decoder_->IsEof() || player_scaler_->IsInitialized())) && // scaler will provide no more frames
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
				if (player_scaler_)
				{
					std::lock_guard<std::mutex> lock(player_scaler_reset_mutex_);
					player_scaler_->Reset();
				}
				std::lock_guard<std::mutex> lock(buffer_content_mutex_);
				if (buffer_)
					buffer_->Loop();
				DebugPrintLine(Common::DebugSeverity::info, "Loop");
			}
			else
			{
				std::lock_guard<std::mutex> lock(buffer_content_mutex_);
				buffer_->Flush();
			}
	}

#pragma endregion


	Core::AVSync PullSync(int audio_samples_count)
	{	
		bool finished = false;
		Core::AVSync sync;
		{
			if (!(buffer_ && buffer_->IsReady()))
			{
				std::unique_lock<std::mutex> lock(buffer_content_mutex_);
				buffer_cv_.wait(lock);
			}
			if (is_eof_)
				return buffer_->PullSync(audio_samples_count);
			sync = buffer_->PullSync(audio_samples_count);
			finished = buffer_->IsEof();
		}
		if (frame_played_callback_)
			frame_played_callback_(sync.TimeInfo);
		if (finished)
		{
			is_eof_ = true;
			Pause();
			DebugPrintLine(Common::DebugSeverity::debug, "Stopped");
		}
		else
		{
			producer_cv_.notify_one();
		}
		return sync;
	}

	bool IsAddedToPlayer(const Core::Player& player)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		return &player == player_;
	}

	void AddToPlayer(const Core::Player& player)
	{
		{
			std::lock_guard<std::mutex> lock(buffer_mutex_);
			if (&player == player_)
			{
				DebugPrintLine(Common::DebugSeverity::error, "Already added to this player");
				return;
			}
			if (player_)
				THROW_EXCEPTION("FFmpegInput: already added to another player");
			player_ = &player;
		}
		producer_cv_.notify_one();
		DebugPrintLine(Common::DebugSeverity::debug, "Added to player");
	}

	void RemoveFromPlayer(const Core::Player& player)
	{
		{
			std::lock_guard<std::mutex> lock(buffer_mutex_);
			if (player_ != &player)
				return;
			player_ = nullptr;
			buffer_.reset();
			player_scaler_.reset();
			audio_muxer_.reset();
		}
		producer_cv_.notify_one();
		DebugPrintLine(Common::DebugSeverity::debug, "Removed from player");
	}


	bool Seek(const std::int64_t time)
	{
		std::lock_guard<std::mutex> lock(buffer_content_mutex_);
		if (input_.Seek(time))
		{
			DebugPrintLine(Common::DebugSeverity::info, "Seek: " + std::to_string(time / 1000));
			if (video_decoder_)
				video_decoder_->Seek(time);
			for (auto& decoder : audio_decoders_)
				decoder->Seek(time);
			if (player_scaler_)
			{
				std::lock_guard<std::mutex> lock(player_scaler_reset_mutex_);
				player_scaler_->Reset();
			}
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

	void Play()
	{
		std::lock_guard<std::mutex> lock(buffer_content_mutex_);
		if (buffer_)
			buffer_->SetIsPlaying(true);
		is_playing_ = true;
	}

	void Pause()
	{
		PAUSED_CALLBACK callback;
		{
			std::lock_guard<std::mutex> lock(buffer_content_mutex_);
			callback = paused_callback_;
			if (buffer_)
				buffer_->SetIsPlaying(false);
			is_playing_ = false;
		}
		if (callback)
			callback();
	}

	void SetIsLoop(bool is_loop)
	{
		is_loop_ = is_loop;
	}

	

	void SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map)
	{

	}

};


FFmpegInput::FFmpegInput(const std::string & file_name, Core::HwAccel acceleration, const std::string& hw_device)
	: impl_(std::make_unique<implementation>(file_name, acceleration, hw_device))
{ }

FFmpegInput::~FFmpegInput(){}
Core::AVSync FFmpegInput::PullSync(const Core::Player& player, int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
bool FFmpegInput::Seek(const std::int64_t time) { return impl_->Seek(time); }
bool FFmpegInput::IsEof() const { return impl_->is_eof_; }
bool FFmpegInput::IsAddedToPlayer(const Core::Player& player) { return impl_->IsAddedToPlayer(player); }
void FFmpegInput::AddToPlayer(const Core::Player& player) { impl_->AddToPlayer(player); }
void FFmpegInput::RemoveFromPlayer(const Core::Player& player) { impl_->RemoveFromPlayer(player);}
void FFmpegInput::AddOutputSink(std::shared_ptr<Core::OutputSink> output_sink) { } //TODO: implement
void FFmpegInput::RemoveOutputSink(std::shared_ptr<Core::OutputSink> output_sink) { }; //TODO: implement
void FFmpegInput::Play() { impl_->Play(); }
void FFmpegInput::Pause() { impl_->Pause(); }
bool FFmpegInput::IsPlaying() const { return impl_->is_playing_; }
void FFmpegInput::SetIsLoop(bool is_loop) { impl_->SetIsLoop(is_loop); }
std::int64_t FFmpegInput::GetAudioDuration() const { return impl_->GetAudioDuration(); }
std::int64_t FFmpegInput::GetVideoStart() const { return impl_->GetVideoStart(); }
std::int64_t FFmpegInput::GetVideoDuration() const { return impl_->GetVideoDuration(); }
AVRational FFmpeg::FFmpegInput::GetTimeBase() const { return impl_->GetTimeBase(); }
AVRational FFmpeg::FFmpegInput::GetFrameRate() const { return impl_->GetFrameRate(); }
int FFmpeg::FFmpegInput::GetWidth() const { return impl_->GetWidth(); }
int FFmpeg::FFmpegInput::GetHeight() const { return impl_->GetHeight(); }
TVPlayR::FieldOrder FFmpeg::FFmpegInput::GetFieldOrder() { return impl_->GetFieldOrder(); }
int FFmpeg::FFmpegInput::GetAudioChannelCount() { return impl_->GetAudioChannelCount(); }
bool FFmpegInput::HaveAlphaChannel() const { return impl_->HaveAlphaChannel(); }
int FFmpegInput::StreamCount() const { return impl_->StreamCount(); }
const Core::StreamInfo& FFmpegInput::GetStreamInfo(int index) const { return impl_->GetStreamInfo(index); }
void FFmpegInput::SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map) { impl_->SetupAudio(audio_channel_map); }
void FFmpegInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
void FFmpegInput::SetPausedCallback(PAUSED_CALLBACK paused_callback) { impl_->paused_callback_ = paused_callback; }
}}