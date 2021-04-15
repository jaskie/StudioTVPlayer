#include "../pch.h"
#include "Utils.h"
#include "../Common/Semaphore.h"
#include "../Common/Executor.h"
#include "FFmpegInputSource.h"
#include "InputFormat.h"
#include "Decoder.h"
#include "../Core/Channel.h"
#include "AudioMuxer.h"
#include "SynchronizingBuffer.h"
#include "ChannelScaler.h"
#include "../Core/AudioChannelMapEntry.h"
#include "../Core/StreamInfo.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace FFmpeg {
			   		 
struct FFmpegInputSource::implementation : Common::DebugTarget<true>
{
	std::atomic_bool is_eof_ = false;
	const std::string file_name_;
	std::atomic_bool is_playing_ = false;
	int64_t seek_time_ = 0LL;
	InputFormat input_;
	const bool is_stream_;
	const Core::HwAccel acceleration_;
	const std::string hw_device_;
	std::unique_ptr<Decoder> video_decoder_;
	std::atomic_bool is_loop_ = false;
	std::vector<std::unique_ptr<Decoder>> audio_decoders_;
	Core::Channel* channel_ = nullptr;
	std::unique_ptr<AudioMuxer> audio_muxer_;
	std::unique_ptr<ChannelScaler> channel_scaler_;
	std::unique_ptr<SynchronizingBuffer> buffer_;
	std::unique_ptr<std::thread> producer_thread_;
	Common::Semaphore producer_semaphore_;
	//std::mutex 
	std::mutex buffer_mutex_;
	std::condition_variable buffer_cv_;
	TIME_CALLBACK frame_played_callback_ = nullptr;
	STOPPED_CALLBACK stopped_callback_ = nullptr;
	LOADED_CALLBACK loaded_callback_ = nullptr;

	implementation(const std::string& fileName, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount)
		: file_name_(fileName)
		, input_(fileName)
		, acceleration_(acceleration)
		, hw_device_(hw_device)
		, is_stream_(IsStream(fileName))
	{ 
		if (seek_time_)
			input_.Seek(seek_time_);
		input_.LoadStreamData();
		InitializeVideoDecoder();
	}

	~implementation()
	{
		RemoveFromChannel();
	}

#pragma region Input thread methods

	void ProducerThreadProc()
	{
		Common::SetThreadName(::GetCurrentThreadId(), ("Input thread for "+file_name_).c_str());
		InitializeAudioDecoders();
		channel_scaler_ = std::make_unique<ChannelScaler>(*video_decoder_, channel_->Format(), PixelFormatToFFmpegFormat(channel_->PixelFormat()));
		if (!audio_decoders_.empty())
			audio_muxer_ = std::make_unique<AudioMuxer>(audio_decoders_, AV_CH_LAYOUT_STEREO, channel_->AudioSampleFormat(), 48000, channel_->AudioChannelsCount());
		buffer_ = std::make_unique<SynchronizingBuffer>(
			channel_->Format(),
			channel_->AudioChannelsCount(),
			channel_->AudioSampleFormat(),
			is_playing_,
			AV_TIME_BASE / 2, // 0.5 sec
			0);
		while (channel_)
		{
			while (!buffer_->Full())
			{
				if (!channel_)
					break;
				ProcessNextInputPacket();
				if (buffer_->Ready())
					buffer_cv_.notify_all();
			}
			producer_semaphore_.wait();
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
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		auto decoded = video_decoder_->Pull();
		if (decoded)
			channel_scaler_->Push(decoded);
		if (!channel_scaler_->IsInitialized())
			return;
		while (auto scaled = channel_scaler_->Pull())
			buffer_->PushVideo(scaled, channel_scaler_->OutputTimeBase());
	}

	void ProcessAudio(const std::unique_ptr<Decoder>& decoder)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		auto decoded = decoder->Pull();
		if (decoded)
			audio_muxer_->Push(decoder->StreamIndex(), decoded);
		while (auto muxed = audio_muxer_->Pull())
			buffer_->PushAudio(muxed);
	}

	void FlushChannelScalerIfNeeded()
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if (video_decoder_->IsEof() && !channel_scaler_->IsFlushed())
			channel_scaler_->Flush();
	}

	void FlushAudioMuxerIfNeeded()
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if (!audio_muxer_ || audio_muxer_->IsFlushed())
			return;
		auto need_flush = std::all_of(audio_decoders_.begin(), audio_decoders_.end(), [](const std::unique_ptr<Decoder>& decoder) { return decoder->IsEof(); });
		if (need_flush)
			audio_muxer_->Flush();
	}

	void FlushBufferOrLoopIfNeeded()
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if (!buffer_->IsFlushed() && // not flushed yet
			(channel_scaler_->IsEof() || (video_decoder_->IsEof() && !channel_scaler_->IsInitialized())) && // scaler will provide no more frames
			(!audio_muxer_ || audio_muxer_->IsEof())) // muxer exists and is Eof
			if (is_loop_)
			{
				input_.Seek(0LL);
				video_decoder_->Seek(0LL);
				for (const auto& decoder : audio_decoders_)
					decoder->Seek(0LL);
				if (audio_muxer_)
					audio_muxer_->Reset();
				if (channel_scaler_)
					channel_scaler_->Reset();
				DebugPrintLine("Loop");
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
			std::unique_lock<std::mutex> lock(buffer_mutex_);
			while (!(buffer_ && buffer_->Ready()))
				buffer_cv_.wait(lock);
			producer_semaphore_.notify();
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
		}
		return sync;
	}

	bool IsAddedToChannel(Core::Channel& channel)
	{
		return &channel == channel_;
	}

	void AddToChannel(Core::Channel& channel)
	{
		if (channel_)
			THROW_EXCEPTION("Already added to another channel");
		channel_ = &channel;
		producer_thread_ = std::make_unique<std::thread>(&implementation::ProducerThreadProc, this);
	}

	void RemoveFromChannel()
	{
		if (!channel_)
			return;
		channel_ = nullptr;
		producer_semaphore_.notify();
		producer_thread_->join();
		producer_thread_.reset();
	}


	bool Seek(const int64_t time)
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		seek_time_ = time;
		if (input_.Seek(time))
		{
			DebugPrintLine(("Seek: " + std::to_string(time / 1000)));
			seek_time_ = time;
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
			producer_semaphore_.notify();
			return true;
		}
		return false;
	}

	void Play()
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		is_playing_ = true;
		if (buffer_)
			buffer_->SetIsPlaying(true);
	}

	void Pause()
	{
		std::lock_guard<std::mutex> lock(buffer_mutex_);
		if(buffer_)
			buffer_->SetIsPlaying(false);
		is_playing_ = false;
		if (stopped_callback_)
			stopped_callback_();
	}

	void SetIsLoop(bool is_loop)
	{
		is_loop_ = is_loop;
	}

	void InitializeVideoDecoder()
	{
		if (video_decoder_)
			return;
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return;
		video_decoder_ = std::make_unique<Decoder>(stream->Codec, stream->Stream, seek_time_, acceleration_, hw_device_);
	}

	void InitializeAudioDecoders()
	{
		auto& streams = input_.GetStreams();
		if (std::any_of(streams.begin(), streams.end(), [](const auto& stream) { return stream.Type == Core::MediaType::audio && stream.Language == "pol"; }))
			for (const auto& stream : streams)
			{
				if (stream.Type == Core::MediaType::audio && stream.Language == "pol")
					audio_decoders_.emplace_back(std::make_unique<Decoder>(stream.Codec, stream.Stream, seek_time_));
			}
		else
			for (const auto& stream : streams)
			{
				if (stream.Type == Core::MediaType::audio)
					audio_decoders_.emplace_back(std::make_unique<Decoder>(stream.Codec, stream.Stream, seek_time_));
			}
	}

	bool IsStream(const std::string& fileName)
	{
		auto prefix = fileName.substr(0, 6);
		return prefix == "udp://" || prefix == "rtp://";
	}

	void SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map)
	{

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
		return PtsToTime(stream->Stream->start_time, stream->Stream->time_base);
	}

	int64_t GetVideoDuration() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return 0LL;
		return PtsToTime(stream->Stream->duration, stream->Stream->time_base);
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

	AVFieldOrder GetFieldOrder() const
	{
		const Core::StreamInfo* stream = input_.GetVideoStream();
		if (stream == nullptr)
			return AVFieldOrder::AV_FIELD_UNKNOWN;
		return stream->Stream->codecpar->field_order;
	}

	int GetAudioChannelCount() const
	{
		return input_.GetTotalAudioChannelCount();
	}

	std::shared_ptr<AVFrame> GetFrameAt(int64_t time)
	{
		if (buffer_)
			return nullptr;
		input_.Seek(time);
		video_decoder_->Seek(time);
		while (!video_decoder_->IsEof())
		{
			auto packet = input_.PullPacket();
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


FFmpegInputSource::FFmpegInputSource(const std::string & file_name, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount)
	: impl_(std::make_unique<implementation>(file_name, acceleration, hw_device, audioChannelCount))
{ }

FFmpegInputSource::~FFmpegInputSource(){}
std::shared_ptr<AVFrame> FFmpegInputSource::GetFrameAt(int64_t time)	{ return impl_->GetFrameAt(time); }
AVSync FFmpegInputSource::PullSync(int audio_samples_count) { return impl_->PullSync(audio_samples_count); }
bool FFmpegInputSource::Seek(const int64_t time)        { return impl_->Seek(time); }
bool FFmpegInputSource::IsEof() const					{ return impl_->is_eof_; }
bool FFmpegInputSource::IsAddedToChannel(Core::Channel& channel) { return impl_->IsAddedToChannel(channel); }
void FFmpegInputSource::AddToChannel(Core::Channel& channel) { impl_->AddToChannel(channel); }
void FFmpegInputSource::RemoveFromChannel()				{ impl_->RemoveFromChannel();}
void FFmpegInputSource::Play()							{ impl_->Play(); }
void FFmpegInputSource::Pause()							{ impl_->Pause(); }
bool FFmpegInputSource::IsPlaying()	const				{ return impl_->is_playing_; }
void FFmpegInputSource::SetIsLoop(bool is_loop) { impl_->SetIsLoop(is_loop); }
int64_t FFmpegInputSource::GetAudioDuration() const		{ return impl_->GetAudioDuration(); }
int64_t FFmpegInputSource::GetVideoStart() const		{ return impl_->GetVideoStart(); }
int64_t FFmpegInputSource::GetVideoDuration() const		{ return impl_->GetVideoDuration(); }
AVRational FFmpeg::FFmpegInputSource::GetTimeBase() const { return impl_->GetTimeBase(); }
AVRational FFmpeg::FFmpegInputSource::GetFrameRate() const { return impl_->GetFrameRate(); }
int FFmpeg::FFmpegInputSource::GetWidth() { return impl_->GetWidth(); }
int FFmpeg::FFmpegInputSource::GetHeight() { return impl_->GetHeight(); }
AVFieldOrder FFmpeg::FFmpegInputSource::GetFieldOrder() { return impl_->GetFieldOrder(); }
int FFmpeg::FFmpegInputSource::GetAudioChannelCount() { return impl_->GetAudioChannelCount(); }
int FFmpegInputSource::StreamCount() const				{ return impl_->StreamCount(); }
Core::StreamInfo& FFmpegInputSource::GetStreamInfo(int index) { return impl_->GetStreamInfo(index); }
void FFmpegInputSource::SetupAudio(const std::vector<Core::AudioChannelMapEntry>& audio_channel_map) { impl_->SetupAudio(audio_channel_map); }
void FFmpegInputSource::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
void FFmpegInputSource::SetStoppedCallback(STOPPED_CALLBACK stopped_callback) { impl_->stopped_callback_ = stopped_callback; }
void FFmpegInputSource::SetLoadedCallback(LOADED_CALLBACK loaded_callback) { impl_->loaded_callback_ = loaded_callback; }

}}