#include "../pch.h"
#include "FFmpegInput.h"
#include "FFmpegInputBase.h"
#include "FFmpegUtils.h"
#include "../Common/Semaphore.h"
#include "../Common/Executor.h"
#include "Decoder.h"
#include "../Core/Channel.h"
#include "AudioMuxer.h"
#include "SynchronizingBuffer.h"
#include "ChannelScaler.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
			   		 
struct FFmpegInput::implementation : Common::DebugTarget, internal::FFmpegInputBase
{
	std::atomic_bool is_eof_ = false;
	std::atomic_bool is_playing_ = false;
	std::atomic_bool is_loop_ = false;
	std::atomic_bool is_producer_running_ = true;
	std::vector<std::unique_ptr<Decoder>> audio_decoders_;
	const Core::Channel* channel_ = nullptr;
	std::unique_ptr<AudioMuxer> audio_muxer_;
	std::unique_ptr<ChannelScaler> channel_scaler_;
	std::unique_ptr<SynchronizingBuffer> buffer_;
	std::mutex buffer_content_mutex_;
	std::mutex buffer_mutex_;
	std::condition_variable buffer_cv_;

	std::thread producer_;
	std::mutex producer_mutex_;
	std::condition_variable producer_cv_;

	TIME_CALLBACK frame_played_callback_ = nullptr;
	STOPPED_CALLBACK stopped_callback_ = nullptr;
	LOADED_CALLBACK loaded_callback_ = nullptr;

	implementation(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device)
		: internal::FFmpegInputBase(file_name, acceleration, hw_device)
		, Common::DebugTarget(false, "FFmpegInput " + file_name)
		, producer_(&implementation::ProducerTheradStart, this)
	{ 
		input_.LoadStreamData();
	}

	~implementation()
	{
		is_producer_running_ = false;
		if (channel_)
			RemoveFromChannel(*channel_);
		producer_cv_.notify_one();
		producer_.join();
	}

#pragma region Input thread methods

	void ProducerTheradStart()
	{
		Common::SetThreadName(::GetCurrentThreadId(), ("FFmpegInput " + file_name_).c_str());
		while (is_producer_running_)
		{
			std::unique_lock<std::mutex> wait_lock(producer_mutex_);
			producer_cv_.wait(wait_lock);
			{
				std::lock_guard<std::mutex> lock(buffer_mutex_);
				if (!channel_)
					continue;
				if (!buffer_ && channel_)
					InitializeBuffer();
				while (!buffer_->IsFull())
				{
					std::lock_guard<std::mutex> lock(buffer_content_mutex_);
					ProcessNextInputPacket();
					if (buffer_->IsReady())
						buffer_cv_.notify_one();
				}
			}			
		}
	}

	void InitializeBuffer()
	{
		InitializeVideoDecoder();
		InitializeAudioDecoders();
		channel_scaler_ = std::make_unique<ChannelScaler>(*channel_);
		if (!audio_decoders_.empty())
			audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, channel_->AudioSampleFormat(), 48000, channel_->AudioChannelsCount());
		buffer_ = std::make_unique<SynchronizingBuffer>(
			channel_,
			is_playing_,
			AV_TIME_BASE / 10, // 100 ms = 2-5 frames
			0);
	}

	void InitializeAudioDecoders()
	{
		auto& streams = input_.GetStreams();
		auto stream = input_.GetVideoStream();
		int64_t seek = stream ? stream->StartTime : 0;
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
				FlushChannelScalerIfNeeded();
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
			channel_scaler_->Push(decoded, video_decoder_->FrameRate(), video_decoder_->TimeBase());
		if (!channel_scaler_->IsInitialized())
			return;
		while (auto scaled = channel_scaler_->Pull())
			buffer_->PushVideo(scaled, channel_scaler_->OutputTimeBase());
	}

	void ProcessAudio(const std::unique_ptr<Decoder>& decoder)
	{
		auto decoded = decoder->Pull();
		if (decoded)
			audio_muxer_->Push(decoder->StreamIndex(), decoded);
		while (auto muxed = audio_muxer_->Pull())
			buffer_->PushAudio(muxed);
	}

	void FlushChannelScalerIfNeeded()
	{
		if (video_decoder_->IsEof() && !channel_scaler_->IsFlushed())
			channel_scaler_->Flush();
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
			(channel_scaler_->IsEof() || (video_decoder_->IsEof() && !channel_scaler_->IsInitialized())) && // scaler will provide no more frames
			(!audio_muxer_ || audio_muxer_->IsEof())) // muxer exists and is Eof
			if (is_loop_)
			{
				int64_t seek_time = input_.GetVideoStream()->StartTime;
				input_.Seek(seek_time);
				video_decoder_->Seek(seek_time);
				for (const auto& decoder : audio_decoders_)
					decoder->Seek(seek_time);
				if (audio_muxer_)
					audio_muxer_->Reset();
				if (channel_scaler_)
					channel_scaler_->Reset();
				if (buffer_)
					buffer_->Loop();
				DebugPrintLine("FFmpegInput: Loop");
			}
			else
				buffer_->Flush();
	}

#pragma endregion


	AVSync PullSync(int audio_samples_count)
	{	
		bool finished = false;
		AVSync sync;
		{
			std::unique_lock<std::mutex> lock(buffer_content_mutex_);
			if (!(buffer_ && buffer_->IsReady()))
				buffer_cv_.wait(lock);
			if (is_eof_)
				return buffer_->PullSync(audio_samples_count);
			sync = buffer_->PullSync(audio_samples_count);
			finished = buffer_->IsEof();
		}
		if (frame_played_callback_)
			frame_played_callback_(sync.Time);
		if (finished)
		{
			is_eof_ = true;
			Pause();
		} else
			producer_cv_.notify_one();
		return sync;
	}

	bool IsAddedToChannel(const Core::Channel& channel)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		return &channel == channel_;
	}

	void AddToChannel(const Core::Channel& channel)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if (&channel == channel_)
		{
			DebugPrintLine("Already added to this channel");
			return;
		}
		if (channel_)
			THROW_EXCEPTION("Already added to another channel");
		channel_ = &channel;
		producer_cv_.notify_one();
	}

	void RemoveFromChannel(const Core::Channel& channel)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if (channel_ != &channel)
			return;
		channel_ = nullptr;
		producer_cv_.notify_one();
	}


	bool Seek(const int64_t time)
	{
		std::lock_guard<std::mutex> lock(buffer_content_mutex_);
		if (input_.Seek(time))
		{
			DebugPrintLine(("Seek: " + std::to_string(time / 1000)));
			if (video_decoder_)
				video_decoder_->Seek(time);
			for (auto& decoder : audio_decoders_)
				decoder->Seek(time);
			if (channel_scaler_)
				channel_scaler_->Reset();
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
		{
			std::lock_guard<std::mutex> lock(buffer_content_mutex_);
			if (buffer_)
				buffer_->SetIsPlaying(false);
			is_playing_ = false;
		}
		if (stopped_callback_)
			stopped_callback_();
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
AVSync FFmpegInput::PullSync(const Core::Channel& channel, int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
bool FFmpegInput::Seek(const int64_t time)        { return impl_->Seek(time); }
bool FFmpegInput::IsEof() const					{ return impl_->is_eof_; }
bool FFmpegInput::IsAddedToChannel(const Core::Channel& channel) { return impl_->IsAddedToChannel(channel); }
void FFmpegInput::AddToChannel(const Core::Channel& channel) { impl_->AddToChannel(channel); }
void FFmpegInput::RemoveFromChannel(const Core::Channel& channel)				{ impl_->RemoveFromChannel(channel);}
void FFmpegInput::AddPreview(std::shared_ptr<Preview::InputPreview> preview)
{
}
void FFmpegInput::Play()							{ impl_->Play(); }
void FFmpegInput::Pause()							{ impl_->Pause(); }
bool FFmpegInput::IsPlaying()	const				{ return impl_->is_playing_; }
void FFmpegInput::SetIsLoop(bool is_loop) { impl_->SetIsLoop(is_loop); }
int64_t FFmpegInput::GetAudioDuration() const		{ return impl_->GetAudioDuration(); }
int64_t FFmpegInput::GetVideoStart() const		{ return impl_->GetVideoStart(); }
int64_t FFmpegInput::GetVideoDuration() const		{ return impl_->GetVideoDuration(); }
AVRational FFmpeg::FFmpegInput::GetTimeBase() const { return impl_->GetTimeBase(); }
AVRational FFmpeg::FFmpegInput::GetFrameRate() const { return impl_->GetFrameRate(); }
int FFmpeg::FFmpegInput::GetWidth() const { return impl_->GetWidth(); }
int FFmpeg::FFmpegInput::GetHeight() const { return impl_->GetHeight(); }
Core::FieldOrder FFmpeg::FFmpegInput::GetFieldOrder() { return impl_->GetFieldOrder(); }
int FFmpeg::FFmpegInput::GetAudioChannelCount() { return impl_->GetAudioChannelCount(); }
bool FFmpegInput::HaveAlphaChannel() const { return impl_->HaveAlphaChannel(); }
int FFmpegInput::StreamCount() const				{ return impl_->StreamCount(); }
const Core::StreamInfo& FFmpegInput::GetStreamInfo(int index) const { return impl_->GetStreamInfo(index); }
void FFmpegInput::SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map) { impl_->SetupAudio(audio_channel_map); }
void FFmpegInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
void FFmpegInput::SetStoppedCallback(STOPPED_CALLBACK stopped_callback) { impl_->stopped_callback_ = stopped_callback; }
void FFmpegInput::SetLoadedCallback(LOADED_CALLBACK loaded_callback) { impl_->loaded_callback_ = loaded_callback; }

}}