#include "../pch.h"
#include "FFmpegInput.h"
#include "FFmpegBufferedInput.h"
#include "FFmpegUtils.h"
#include "../Core/AVSync.h"
#include "../Core/Player.h"
#include "PlayerScaler.h"
#include "FFmpegInputPlayerSource.h"


namespace TVPlayR {
	namespace FFmpeg {

struct FFmpegInput::implementation : FFmpegBufferedInput
{
	std::atomic_bool is_eof_ = false;
	std::atomic_bool is_playing_ = false;
	std::atomic_bool is_loop_ = false;
	std::atomic_bool is_running_ = true;
	std::unique_ptr<FFmpegInputPlayerSource> player_source_;
	std::mutex player_scaler_reset_mutex_;

	TIME_CALLBACK frame_played_callback_ = nullptr;
	PAUSED_CALLBACK paused_callback_ = nullptr;
	std::thread frame_reader_thread_;

	implementation(const std::string &file_name, Core::HwAccel acceleration, const std::string &hw_device)
		: FFmpegBufferedInput(file_name, acceleration, hw_device, Core::AudioParameters { 48000, 2, AVSampleFormat::AV_SAMPLE_FMT_FLT})
	{
	}

	~implementation()
	{
		ReleaseThreads();
	}

	Core::AVSync PullSync(const Core::Player &player, int audio_samples_count)
	{
		Core::AVSync sync = player_source_->PullSync(audio_samples_count);
		if (frame_played_callback_)
			frame_played_callback_(sync.TimeInfo);
		if (is_eof_)
		{
			Pause();
		}
		return sync;
	}

	bool IsAddedToPlayer(const Core::Player &player)
	{
		return player_source_ && &player == &player_source_->Player();
	}

	void AddToPlayer(const Core::Player &player)
	{
		std::lock_guard<std::mutex> lock(player_scaler_reset_mutex_);
		if (player_source_ && &player == &player_source_->Player())
		{
			DebugPrintLine(Common::DebugSeverity::error, "Already added to this player");
			return;
		}
		if (player_source_)
			THROW_EXCEPTION("FFmpegInput: already added to another player");
		player_source_ = std::make_unique<FFmpegInputPlayerSource>(player, true, player.AudioChannelsCount());
		is_running_ = true;
		frame_reader_thread_ = std::thread(&implementation::ProducerTheradStart, this);
	}

	void RemoveFromPlayer(const Core::Player &player)
	{
		if (!player_source_ || &player_source_->Player() != &player)
			return;
		ReleaseThreads();
	}

	void Play()
	{
		is_playing_ = true;
	}

	void Pause()
	{
		is_playing_ = false;
		if (paused_callback_)
			paused_callback_();
	}

	bool Seek(const std::int64_t time)
	{
		return true;
	}

	void SetIsLoop(bool is_loop)
	{
		is_loop_ = is_loop;
	}

	void SetupAudio(const std::vector<Core::AudioChannelMapEntry> &audio_channel_map)
	{

	}

	void ProducerTheradStart()
	{
		while (is_running_)
		{
			auto sync = FFmpegBufferedInput::PullSync();
			player_source_->Push(sync, FFmpegInputBase::GetFrameRate());
		}
	}

	void ReleaseThreads() override final
	{
		FFmpegBufferedInput::ReleaseThreads();
		is_running_ = false;
		player_source_->Release();
		if (frame_reader_thread_.joinable())
			frame_reader_thread_.join();
		player_source_.reset();
	}

};


FFmpegInput::FFmpegInput(const std::string &file_name, Core::HwAccel acceleration, const std::string &hw_device)
	: impl_(std::make_unique<implementation>(file_name, acceleration, hw_device))
{ }

FFmpegInput::~FFmpegInput(){}
Core::AVSync FFmpegInput::PullSync(const Core::Player &player, int audio_samples_count) { return impl_->PullSync(player, audio_samples_count); }
bool FFmpegInput::Seek(const std::int64_t time) { return impl_->Seek(time); }
bool FFmpegInput::IsEof() const { return impl_->is_eof_; }
bool FFmpegInput::IsAddedToPlayer(const Core::Player &player) { return impl_->IsAddedToPlayer(player); }
void FFmpegInput::AddToPlayer(const Core::Player &player) { impl_->AddToPlayer(player); }
void FFmpegInput::RemoveFromPlayer(const Core::Player &player) { impl_->RemoveFromPlayer(player);}
void FFmpegInput::AddOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) { } //TODO: implement
void FFmpegInput::RemoveOutputSink(std::shared_ptr<Core::OutputSink> &output_sink) { }; //TODO: implement
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
void FFmpegInput::SetupAudio(const std::vector<Core::AudioChannelMapEntry> &audio_channel_map) { impl_->SetupAudio(audio_channel_map); }
void FFmpegInput::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
void FFmpegInput::SetPausedCallback(PAUSED_CALLBACK paused_callback) { impl_->paused_callback_ = paused_callback; }
}}