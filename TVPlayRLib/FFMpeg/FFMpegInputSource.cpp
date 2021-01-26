#include "../pch.h"
#include "Utils.h"
#include "FFmpegInputSource.h"
#include "InputFormat.h"
#include "Decoder.h"
#include "../Core/OutputDeviceSource.h"
#include "AudioFilter.h"

namespace TVPlayR {
	namespace FFmpeg {


static std::vector<std::unique_ptr<Decoder>> create_decoders(AVCodec * const codec, const std::vector<AVStream*> streams, int64_t fifo_duration)
{
	std::vector<std::unique_ptr<Decoder>> ret(streams.size());
	std::transform(streams.begin(), streams.end(), ret.begin(), [&](AVStream* stream) {
		return std::unique_ptr<Decoder>(new Decoder(codec, stream, fifo_duration));
	});
	return ret;
}

struct FFmpegInputSource::implementation
{
	std::mutex mutex_;
	bool is_eof_ = false;
	bool is_playing_ = false;
	Core::HwAccel _acceleration;
	InputFormat input_;
	Decoder video_decoder_;
	const int64_t frame_duration_;
	std::vector<std::unique_ptr<Decoder>> audio_decoders_;
	std::vector<Core::OutputDeviceSource*> channels_;
	AVFramePtr last_frame_;
	std::unique_ptr<AudioFilter> audio_filter_;
	TIME_CALLBACK frame_played_callback_ = nullptr;
	STOPPED_CALLBACK stopped_callback_ = nullptr;

	implementation(const std::string& fileName, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount)
		: _acceleration(acceleration)
		, input_(fileName)
		, frame_duration_(av_rescale(AV_TIME_BASE, video_decoder_.frame_rate.den, video_decoder_.frame_rate.num))
		, audio_decoders_(audioChannelCount > 0 ? create_decoders(input_.GetAudioCodec(), input_.GetAudioStreams(), AV_TIME_BASE) : std::vector<std::unique_ptr<Decoder>>()) //fifo for one second
		, video_decoder_(input_.GetVideoCodec(), input_.GetVideoStream(), acceleration, hw_device)
	{ }

	FFmpeg::AVFramePtr PullVideo()
	{
		std::lock_guard<std::mutex> guard(mutex_);
		if (is_playing_ || !last_frame_)
		{
			while (video_decoder_.Empty())
				if (!DecodeNextFrame())
				{
					if (stopped_callback_)
						stopped_callback_();
					return nullptr;
				}
			last_frame_ = video_decoder_.PullVideo();
			for (auto& decoder : audio_decoders_)
			{
				auto frame = decoder->PullAudio();
				if (frame && audio_filter_)
					audio_filter_->Push(decoder->stream_index, frame);
			}
			if (frame_played_callback_ && last_frame_)
				frame_played_callback_(video_decoder_.TimeFromTs(last_frame_->pts));
		}
		return last_frame_;
	}

	FFmpeg::AVFramePtr LastVideo()
	{
		auto last_frame = last_frame_;
		if (!last_frame)
			return PullVideo();
		return last_frame;
	}

	FFmpeg::AVFramePtr PullAudio(int audio_samples_count)
	{
		std::lock_guard<std::mutex> guard(mutex_);
		if (!is_playing_ || !audio_filter_)
			return nullptr;
		return audio_filter_->Pull(audio_samples_count);
	}

	void AddToOutput(Core::OutputDeviceSource* source)
	{
		channels_.push_back(source);
		SetupAudio(source->AudioChannelsCount());
	}

	void RemoveFromOutput(Core::OutputDeviceSource* source)
	{
		channels_.erase(std::remove(channels_.begin(), channels_.end(), source), channels_.end());
	}

	void SetupAudio(int channels)
	{
		audio_filter_.reset(new AudioFilter(audio_decoders_, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S32, 48000, 2));
	}

	bool DecodeNextFrame()
	{
		if (is_eof_)
			return false;
		while (true)
		{
			if (!PushNextPacket())
				return false;
			if (!video_decoder_.Empty())
			{
				bool need_more_audio = false;
				int64_t video_end_time = video_decoder_.FrontFrameEndTime();
				for (auto& audio_decoder : audio_decoders_)
				{
					int64_t audio_max_time = audio_decoder->TimeMax();
					if (!audio_decoder->IsFlushed() && audio_max_time < video_end_time)
					{
						need_more_audio = true;
						break;
					}
				}
				if (!need_more_audio)
					return true;
			}
			else
			{
				if (video_decoder_.IsEof())
				{
					is_eof_ = true;
					return false;
				}
			}
		}
		return false;
	}

	bool PushNextPacket()
	{
		auto packet = input_.PullPacket();
		if (!packet)
		{
			if (input_.IsEof() && !video_decoder_.IsFlushed())
			{
				for (auto& decoder : audio_decoders_)
					decoder->Flush();
				video_decoder_.Flush();
			}
			else
				return false;
		}
		else
		{
			if (packet->stream_index == video_decoder_.stream_index)
				video_decoder_.PushPacket(packet);
			else
				for (auto& audio_decoder : audio_decoders_)
				{
					if (packet->stream_index == audio_decoder->stream_index)
					{
						audio_decoder->PushPacket(packet);
						break;
					}
				}
		}
		return true;
	}


	bool Seek(const int64_t time)
	{
		std::lock_guard<std::mutex> guard(mutex_);
		auto dur = input_.GetVideoDuration();
		if (time > dur)
			return false;
		if (input_.Seek(time))
		{
			OutputDebugStringA(("Seek: " + std::to_string(time / 1000) + "\n").c_str());
			video_decoder_.Seek(time);
			for (auto& decoder : audio_decoders_)
				decoder->Seek(time);
			if (audio_filter_) 
				audio_filter_->Reset();
			is_eof_ = false;
			last_frame_.reset();
			for (auto output : channels_)
				output->ResetFilter();
			return true;
		}
		return false;
	}

	void Play()
	{
		is_playing_ = true;
	}

	void Pause()
	{
		is_playing_ = false;
		if (stopped_callback_)
			stopped_callback_();
	}

	AVRational GetTimeBase() const
	{
		return video_decoder_.stream->time_base;
	}

	AVRational GetFrameRate() const
	{
		return video_decoder_.stream->r_frame_rate;
	}

	int64_t GetVideoDecoderTime()
	{
		return video_decoder_.TimeMax();
	}
};

}
FFmpeg::FFmpegInputSource::FFmpegInputSource(const std::string & file_name, Core::HwAccel acceleration, const std::string& hw_device, int audioChannelCount)
	: impl_(new implementation(file_name, acceleration, hw_device, audioChannelCount))
{ }

FFmpeg::FFmpegInputSource::~FFmpegInputSource(){}
FFmpeg::AVFramePtr FFmpeg::FFmpegInputSource::PullVideo()       { return impl_->PullVideo(); }
FFmpeg::AVFramePtr FFmpeg::FFmpegInputSource::LastVideo()		{ return impl_->LastVideo(); }
FFmpeg::AVFramePtr FFmpeg::FFmpegInputSource::PullAudio(int audio_samples_count)       { return impl_->PullAudio(audio_samples_count); }
bool FFmpeg::FFmpegInputSource::Seek(const int64_t time)        { return impl_->Seek(time); }
AVRational FFmpeg::FFmpegInputSource::GetTimeBase() const       { return impl_->GetTimeBase(); }
AVRational FFmpeg::FFmpegInputSource::GetFrameRate() const      { return impl_->GetFrameRate(); }
bool FFmpeg::FFmpegInputSource::IsEof() const					{ return impl_->is_eof_; }
void FFmpeg::FFmpegInputSource::AddToOutput(Core::OutputDeviceSource * source)		{ impl_->AddToOutput(source); }
void FFmpeg::FFmpegInputSource::RemoveFromOutput(Core::OutputDeviceSource * source)	{ impl_->RemoveFromOutput(source);}
void FFmpeg::FFmpegInputSource::SetupAudio(int channels)		{ impl_->SetupAudio(channels); }
int64_t FFmpeg::FFmpegInputSource::GetVideoDuration()			{ return impl_->input_.GetVideoDuration(); }
int64_t FFmpeg::FFmpegInputSource::GetAudioDuration()			{ return impl_->input_.GetAudioDuration(); }
int64_t FFmpeg::FFmpegInputSource::GetVideoDecoderTime()		{ return impl_->GetVideoDecoderTime(); }
void FFmpeg::FFmpegInputSource::Play()							{ impl_->Play(); }
void FFmpeg::FFmpegInputSource::Pause()							{ impl_->Pause(); }
bool FFmpeg::FFmpegInputSource::IsPlaying()	const				{ return impl_->is_playing_; }
void FFmpeg::FFmpegInputSource::SetFramePlayedCallback(TIME_CALLBACK frame_played_callback) { impl_->frame_played_callback_ = frame_played_callback; }
void FFmpeg::FFmpegInputSource::SetStoppedCallback(STOPPED_CALLBACK stopped_callback) { impl_->stopped_callback_ = stopped_callback; }

}