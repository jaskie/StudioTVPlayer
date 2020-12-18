#include "../pch.h"
#include "Decklink.h"
#include "../Common/ComInitializer.h"
#include "../Common/Executor.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../Core/OutputFrameClock.h"
#include "AVDecklinkVideoFrame.h"
#include "AVDecklinkAudioBuffer.h"

namespace TVPlayR {
	namespace Decklink {

		static BMDDisplayMode GetDecklinkVideoFormat(Core::VideoFormat::Type fmt)
		{
			switch (fmt)
			{
			case Core::VideoFormat::pal:
			case Core::VideoFormat::pal_fha:
				return bmdModePAL;
			case Core::VideoFormat::ntsc:
			case Core::VideoFormat::ntsc_fha:
				return bmdModeNTSC;
			case Core::VideoFormat::v1080p2398:		return bmdModeHD1080p2398;
			case Core::VideoFormat::v1080p2400:		return bmdModeHD1080p24;
			case Core::VideoFormat::v1080i5000:		return bmdModeHD1080i50;
			case Core::VideoFormat::v1080i5994:		return bmdModeHD1080i5994;
			case Core::VideoFormat::v1080i6000:		return bmdModeHD1080i6000;
			case Core::VideoFormat::v1080p2500:		return bmdModeHD1080p25;
			case Core::VideoFormat::v1080p2997:		return bmdModeHD1080p2997;
			case Core::VideoFormat::v1080p3000:		return bmdModeHD1080p30;
			case Core::VideoFormat::v1080p5000:		return bmdModeHD1080p50;
			case Core::VideoFormat::v1080p5994:		return bmdModeHD1080p5994;
			case Core::VideoFormat::v1080p6000:		return bmdModeHD1080p6000;
			case Core::VideoFormat::v2160p2398:		return bmdMode4K2160p2398;
			case Core::VideoFormat::v2160p2400:		return bmdMode4K2160p24;
			case Core::VideoFormat::v2160p2500:		return bmdMode4K2160p25;
			case Core::VideoFormat::v2160p2997:		return bmdMode4K2160p2997;
			case Core::VideoFormat::v2160p3000:		return bmdMode4K2160p30;
			case Core::VideoFormat::v2160p5000:		return bmdMode4K2160p50;
			case Core::VideoFormat::v2160p6000:		return bmdMode4K2160p60;
			default:
				return (BMDDisplayMode)ULONG_MAX;
			}
		}

		static BMDPixelFormat BMDPixelFormatFromVideoFormat(const Core::PixelFormat& format)
		{
			switch (format)
			{
			case Core::PixelFormat::bgra:
				return BMDPixelFormat::bmdFormat8BitBGRA;
			case Core::PixelFormat::yuv422:
				return BMDPixelFormat::bmdFormat8BitYUV;
			default:
				break;
			}
			return (BMDPixelFormat)0;
		}

		struct Decklink::implementation : IDeckLinkVideoOutputCallback
		{
			Common::ComInitializer com_;
			const CComPtr<IDeckLink> decklink_;
			const CComQIPtr<IDeckLinkOutput> output_;
			CComPtr<IDeckLinkMutableVideoFrame> empty_video_frame_;
			std::deque<std::pair<AVDecklinkVideoFrame, AVDecklinkAudioBuffer>> frame_buffer_;
			Core::VideoFormat format_;
			BMDPixelFormat pixel_format_ = BMDPixelFormat::bmdFormat8BitYUV;
			size_t buffer_size_ = 8;
			int64_t scheduled_frames_ = 0ULL;
			int64_t scheduled_samples_ = 0ULL;
			std::shared_ptr<Core::OutputFrameClock> output_frame_clock_;
			Common::Executor executor_;
			std::mutex mutex_;

			implementation(IDeckLink* decklink)
				: decklink_(decklink)
				, output_(decklink)
				, format_(Core::VideoFormat::invalid)
				, executor_()
			{
				if (output_)
					output_frame_clock_.reset(new Core::OutputFrameClock());
			}

			~implementation()
			{
				ReleaseChannel();
			}

			bool OpenOutput(BMDDisplayMode mode, BMDPixelFormat pixel_format)
			{
				if (!output_)
					return false;
				BMDDisplayModeSupport modeSupport;
				if (output_->DoesSupportVideoMode(mode, BMDPixelFormat::bmdFormat8BitYUV, BMDVideoOutputFlags::bmdVideoOutputFlagDefault, &modeSupport, NULL) != S_OK
					|| modeSupport == bmdDisplayModeNotSupported)
					return false;
				if (output_->EnableVideoOutput(mode, BMDVideoOutputFlags::bmdVideoOutputFlagDefault) != S_OK)
					return false;
				output_->EnableAudioOutput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, BMDAudioSampleType::bmdAudioSampleType32bitInteger, 2, BMDAudioOutputStreamType::bmdAudioOutputStreamContinuous);
				return true;
			}
						
			void Preroll()
			{
				if (IsPlaying() || !output_)
					return;
				scheduled_frames_ = 0LL;
				scheduled_samples_ = 0LL;
				output_->BeginAudioPreroll();
				for (size_t i = 0; i < buffer_size_; i++)
				{
					ScheduleVideo(*empty_video_frame_);
					AVDecklinkAudioBuffer audio(AudioSamplesRequired(), 2);
					ScheduleAudio(audio);
				}
				output_->EndAudioPreroll();
			}

			void ScheduleVideo(IDeckLinkVideoFrame& frame)
			{
				int64_t frame_time = scheduled_frames_ * format_.FrameRate().denominator();
				output_->ScheduleVideoFrame(&frame, frame_time, format_.FrameRate().denominator(), format_.FrameRate().numerator());
				scheduled_frames_++;
			}

			void ScheduleAudio(AVDecklinkAudioBuffer& buffer)
			{
				if (!buffer)
					return;
				unsigned int samples_written;
				output_->ScheduleAudioSamples(buffer.GetBytes(), buffer.SamplesCount(), scheduled_samples_, 48000LL, &samples_written);
				scheduled_samples_ += buffer.SamplesCount();
			}

			int AudioSamplesRequired() const
			{
				return static_cast<int>(av_rescale(scheduled_frames_, 48000LL * format_.FrameRate().denominator(), format_.FrameRate().numerator()) - scheduled_samples_);
			}

			CComPtr<IDeckLinkMutableVideoFrame> CreateEmptyVideoFrame()
			{
				CComPtr<IDeckLinkMutableVideoFrame> frame;
				int bpp;
				uint32_t black_pattern;
				switch (pixel_format_)
				{
				case bmdFormat8BitYUV:
					bpp = 2;
					black_pattern = 0x10801080;
					break;
				default:
					bpp = 4;
					black_pattern = 0x10101000;
					break;
				}
				if (output_->CreateVideoFrame(format_.width(), format_.height(), format_.width() * bpp, pixel_format_, bmdFrameFlagDefault, &frame) != S_OK)
					return nullptr;

				uint32_t* nextWord;
				frame->GetBytes((void**)&nextWord);
				long wordsRemaining = (frame->GetRowBytes() * frame->GetHeight()) / sizeof(uint32_t);
				while (wordsRemaining-- > 0)
					*(nextWord++) = black_pattern;
				return frame;
			}


			std::shared_ptr<Core::OutputFrameClock> OutputFrameClock()
			{
				return output_frame_clock_;
			}

			std::wstring GetDisplayName()
			{
				BSTR pModelName;
				if (decklink_->GetDisplayName(&pModelName) != S_OK)
					return L"";
				return std::wstring(pModelName);
			}

			std::wstring GetModelName()
			{
				BSTR pModelName;
				if (decklink_->GetModelName(&pModelName) != S_OK)
					return L"";
				return std::wstring(pModelName);
			}

			bool AssignToChannel(Core::Channel* channel)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				BMDPixelFormat pixel_format = BMDPixelFormatFromVideoFormat(channel->PixelFormat());
				if (!OpenOutput(GetDecklinkVideoFormat(channel->Format().type()), pixel_format))
					return false;
				format_ = channel->Format();
				pixel_format_ = pixel_format;
				empty_video_frame_ = CreateEmptyVideoFrame();
				Preroll();
				output_->SetScheduledFrameCompletionCallback(this);
				output_->StartScheduledPlayback(0LL, format_.FrameRate().numerator(), 1.0);
				return true;
			}

			void ReleaseChannel()
			{
				BOOL is_playing;
				if (output_->IsScheduledPlaybackRunning(&is_playing) != S_OK || !is_playing)
					return;
				{
					std::lock_guard<std::mutex> lock(mutex_);
					output_->SetScheduledFrameCompletionCallback(nullptr);
					BMDTimeValue frame_time = scheduled_frames_ * format_.FrameRate().denominator();
					BMDTimeValue actual_stop;
					output_->StopScheduledPlayback(frame_time, &actual_stop, format_.FrameRate().numerator());
					output_->DisableAudioOutput();
					output_->DisableVideoOutput();
					frame_buffer_.clear();
					format_ = Core::VideoFormat::invalid;
				}
			}

			bool IsPlaying() const
			{
				BOOL isPlaying;
				if (output_ && output_->IsScheduledPlaybackRunning(&isPlaying) == S_OK && isPlaying)
					return true;
				return false;
			}

			void Push(FFmpeg::AVFramePtr& video, FFmpeg::AVFramePtr& audio)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				frame_buffer_.emplace_back(std::pair<AVDecklinkVideoFrame, AVDecklinkAudioBuffer>(video, audio));
			}

			bool SetBufferSize(size_t size)
			{
				if (IsPlaying())
					return false;
				buffer_size_ = size;
				return true;
			}

			size_t GetBufferSize() const
			{
				return buffer_size_;
			}

			//IDeckLinkVideoOutputCallback
			HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame * completedFrame, BMDOutputFrameCompletionResult result) override
			{
				int required_samples = AudioSamplesRequired();
				if (output_frame_clock_)
					output_frame_clock_->RequestFrame(required_samples);
				std::lock_guard<std::mutex> lock(mutex_);
				if (frame_buffer_.empty())
				{
					ScheduleVideo(*empty_video_frame_);
					AVDecklinkAudioBuffer audio(required_samples, 2);
					ScheduleAudio(audio);
				}
				else
				{
					ScheduleVideo(frame_buffer_.front().first);
					ScheduleAudio(frame_buffer_.front().second);
					frame_buffer_.pop_front();
				}
				return S_OK;
			}

			HRESULT __stdcall ScheduledPlaybackHasStopped(void) override
			{
				return S_OK;
			}

			//IUnknown
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
			ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
			ULONG STDMETHODCALLTYPE Release() override { return 1; }

		};

		Decklink::Decklink(IDeckLink * decklink)
			: impl_(new implementation(decklink))
		{}

		Decklink::~Decklink() { }

		std::shared_ptr<Core::OutputFrameClock> Decklink::OutputFrameClock() { return impl_->OutputFrameClock();	}
		std::wstring Decklink::GetDisplayName() { return impl_->GetDisplayName(); }
		std::wstring Decklink::GetModelName() { return impl_->GetModelName(); }
		bool Decklink::SetBufferSize(size_t size) { return impl_->SetBufferSize(size); }
		size_t Decklink::GetBufferSize() const { return impl_->GetBufferSize(); }
		bool Decklink::AssignToChannel(Core::Channel* channel) { 
			if (Core::OutputDevice::AssignToChannel(channel)
				&& impl_->AssignToChannel(channel))
				return true;
			Core::OutputDevice::ReleaseChannel();
			return false;
		}

		void Decklink::ReleaseChannel()
		{
			impl_->ReleaseChannel();
			Core::OutputDevice::ReleaseChannel();
		}

		bool Decklink::IsPlaying() const { return impl_->IsPlaying(); }
		void Decklink::Push(FFmpeg::AVFramePtr & video, FFmpeg::AVFramePtr& audio) { impl_->Push(video, audio);	}
	}
}

