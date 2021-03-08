#include "../pch.h"
#include "Decklink.h"
#include "../Common/ComInitializer.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"
#include "../Core/Channel.h"
#include "../Common/Executor.h"
#include "Utils.h"
#include "DecklinkVideoFrame.h"

//#undef DEBUG

namespace TVPlayR {
	namespace Decklink {
		
		struct Decklink::implementation : IDeckLinkVideoOutputCallback
		{
			const CComPtr<IDeckLink> decklink_;
			const CComQIPtr<IDeckLinkOutput> output_;
			int index_;
			Core::VideoFormat format_;
			int buffer_size_ = 4;
			int64_t scheduled_frames_ = 0;
			int64_t scheduled_samples_ = 0;
			int audio_channels_count_ = 2;
			std::atomic_bool is_running_;
			FRAME_REQUESTED_CALLBACK frame_requested_callback_ = nullptr;

			implementation(IDeckLink* decklink, int index)
				: decklink_(decklink)
				, output_(decklink)
				, format_(Core::VideoFormatType::invalid)
				, index_(index)
			{
				output_->SetScheduledFrameCompletionCallback(this);
			}

			~implementation()
			{
				output_->SetScheduledFrameCompletionCallback(nullptr);
				frame_requested_callback_ = nullptr;
				ReleaseChannel();
			}

			bool OpenOutput(BMDDisplayMode mode, BMDPixelFormat pixel_format, int audio_channels_count)
			{
				if (!output_)
					return false;
				BMDDisplayModeSupport modeSupport;
				if (output_->DoesSupportVideoMode(mode, BMDPixelFormat::bmdFormat8BitYUV, BMDVideoOutputFlags::bmdVideoOutputFlagDefault, &modeSupport, NULL) != S_OK
					|| modeSupport == bmdDisplayModeNotSupported)
					return false;
				if (output_->EnableVideoOutput(mode, BMDVideoOutputFlags::bmdVideoOutputFlagDefault) != S_OK)
					return false;
				output_->EnableAudioOutput(BMDAudioSampleRate::bmdAudioSampleRate48kHz, BMDAudioSampleType::bmdAudioSampleType32bitInteger, audio_channels_count, BMDAudioOutputStreamType::bmdAudioOutputStreamContinuous);
				return true;
			}
						
			void Preroll(Core::Channel& channel)
			{
				if (is_running_ || !output_)
					return;
				scheduled_frames_ = 0LL;
				scheduled_samples_ = 0LL;
				output_->BeginAudioPreroll();
				auto empty_video_frame = FFmpeg::CreateEmptyVideoFrame(channel.Format(), channel.PixelFormat());
				for (size_t i = 0; i < buffer_size_; i++)
				{
					ScheduleAudio(FFmpeg::CreateSilentAudioFrame(AudioSamplesRequired(), audio_channels_count_));
					ScheduleVideo(empty_video_frame);
				}
				output_->EndAudioPreroll();
			}

			bool ScheduleVideo(const std::shared_ptr<AVFrame>& frame)
			{
				int64_t frame_time = scheduled_frames_ * format_.FrameRate().Denominator();

				DecklinkVideoFrame* decklink_frame = new DecklinkVideoFrame(frame);
				HRESULT ret = output_->ScheduleVideoFrame(decklink_frame, frame_time, format_.FrameRate().Denominator(), format_.FrameRate().Numerator());
				scheduled_frames_++;
#ifdef DEBUG
				std::stringstream msg;
				msg << "scheduled frame: " << frame->pts << "\n";
				OutputDebugStringA(msg.str().c_str());
#endif			
				return ret == S_OK;
			}

			void ScheduleAudio(const std::shared_ptr<AVFrame>& buffer)
			{
				unsigned int samples_written;
				output_->ScheduleAudioSamples(buffer->data[0], buffer->nb_samples, scheduled_samples_, BMDAudioSampleRate::bmdAudioSampleRate48kHz, &samples_written);
				scheduled_samples_ += buffer->nb_samples;
#ifdef DEBUG
				std::stringstream msg;
				msg << "Audio samples written: " << samples_written << " from buffer of " << buffer->nb_samples <<"\n";
				OutputDebugStringA(msg.str().c_str());
#endif	
			}

			int AudioSamplesRequired() const
			{
				return static_cast<int>(av_rescale(scheduled_frames_ + 1, BMDAudioSampleRate::bmdAudioSampleRate48kHz * format_.FrameRate().Denominator(), format_.FrameRate().Numerator()) - scheduled_samples_);
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

			bool SetBufferSize(int size) 
			{
				if (is_running_)
					return false;
				buffer_size_ = size;
				return true;
			}

			bool AssignToChannel(Core::Channel& channel)
			{
				if (!OpenOutput(GetDecklinkVideoFormat(channel.Format().type()), BMDPixelFormatFromVideoFormat(channel.PixelFormat()), channel.AudioChannelsCount()))
					return false;
				format_ = channel.Format();
				audio_channels_count_ = channel.AudioChannelsCount();
				Preroll(channel);
				output_->StartScheduledPlayback(0LL, format_.FrameRate().Numerator(), 1.0);
				is_running_ = true;
				return true;
			}

			void ReleaseChannel()
			{
				if (!is_running_.exchange(false))
					return;
				BMDTimeValue frame_time = scheduled_frames_ * format_.FrameRate().Denominator();
				BMDTimeValue actual_stop;
				output_->StopScheduledPlayback(frame_time, &actual_stop, format_.FrameRate().Numerator());
				output_->DisableAudioOutput();
				output_->DisableVideoOutput();
				format_ = Core::VideoFormatType::invalid;
			}

			void Push(FFmpeg::AVSync& sync)
			{
				ScheduleVideo(sync.Video);
				ScheduleAudio(sync.Audio);
			}

			//IDeckLinkVideoOutputCallback
			HRESULT __stdcall ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override
			{
				if (frame_requested_callback_)
					frame_requested_callback_(AudioSamplesRequired());

#ifdef DEBUG
				auto frame = static_cast<DecklinkVideoFrame*>(completedFrame);
				if (result != BMDOutputFrameCompletionResult::bmdOutputFrameCompleted)
				{
					std::stringstream msg;
					msg << "Frame: " << scheduled_frames_ << ": " << ((result == BMDOutputFrameCompletionResult::bmdOutputFrameDisplayedLate) ? "late" : "dropped") << "\n";
					OutputDebugStringA(msg.str().c_str());
				}
				else
				{
					std::stringstream msg;
					msg << "Frame: " << scheduled_frames_ << ": " << frame->GetPts() << "\n";
					OutputDebugStringA(msg.str().c_str());
				}
#endif
				//delete frame;
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

		Decklink::Decklink(IDeckLink * decklink, int index)
			: impl_(std::make_unique<implementation>(decklink, index))
		{}

		Decklink::~Decklink() { }

		std::wstring Decklink::GetDisplayName() { return impl_->GetDisplayName(); }
		std::wstring Decklink::GetModelName() { return impl_->GetModelName(); }
		bool Decklink::SetBufferSize(int size) { return impl_->SetBufferSize(size); }
		int Decklink::GetBufferSize() const { return impl_->buffer_size_; }
		bool Decklink::AssignToChannel(Core::Channel& channel) { 
			return Core::OutputDevice::AssignToChannel(channel)
				&& impl_->AssignToChannel(channel);
		}

		void Decklink::ReleaseChannel()	{ impl_->ReleaseChannel(); }
		bool Decklink::IsPlaying() const { return impl_->is_running_; }
		void Decklink::Push(FFmpeg::AVSync& sync) { impl_->Push(sync); }
		void Decklink::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) { impl_->frame_requested_callback_ = frame_requested_callback; }
	}
}

