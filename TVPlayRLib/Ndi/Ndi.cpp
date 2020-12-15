#include "../pch.h"
#include "Ndi.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "../Core/OutputFrameClock.h"

namespace TVPlayR {
	namespace Ndi {

		struct Ndi::implementation
		{
			Core::VideoFormat format_;
			std::shared_ptr<Core::OutputFrameClock> output_frame_clock_;
			const std::string source_name_;
			const std::string group_name_;
			std::deque<FFmpeg::AVFramePtr> video_frame_buffer_;
			std::deque<FFmpeg::AVFramePtr> audio_frame_buffer_;

			implementation(const std::string& source_name, const std::string& group_name)
				: format_(Core::VideoFormat::invalid)
			{
				output_frame_clock_.reset(new Core::OutputFrameClock());
			}

			~implementation()
			{
				ReleaseChannel();
			}

			bool OpenOutput(BMDDisplayMode mode, BMDPixelFormat pixel_format)
			{
				return true;
			}

			std::shared_ptr<Core::OutputFrameClock> OutputFrameClock()
			{
				return output_frame_clock_;
			}


			bool AssignToChannel(Core::Channel * channel)
			{
				return true;
			}

			void ReleaseChannel()
			{

			}

			bool IsPlaying() const
			{
				return false;
			}

			void Push(FFmpeg::AVFramePtr& video, FFmpeg::AVFramePtr & audio)
			{
				video_frame_buffer_.emplace_back(video);
				audio_frame_buffer_.emplace_back(audio);
			}

		};
			
		Ndi::Ndi(const std::string& source_name, const std::string& group_name) : impl_(new implementation(source_name, group_name)) { }
		Ndi::~Ndi() { }
		std::shared_ptr<Core::OutputFrameClock> Ndi::OutputFrameClock() { return impl_->OutputFrameClock();	}
		
		bool Ndi::AssignToChannel(Core::Channel * channel) { 
			if (Core::OutputDevice::AssignToChannel(channel)
				&& impl_->AssignToChannel(channel))
				return true;
			Core::OutputDevice::ReleaseChannel();
			return false;
		}

		void Ndi::ReleaseChannel()
		{
			impl_->ReleaseChannel();
			Core::OutputDevice::ReleaseChannel();
		}

		bool Ndi::IsPlaying() const { return impl_->IsPlaying(); }
		void Ndi::Push(FFmpeg::AVFramePtr & video, FFmpeg::AVFramePtr& audio) { impl_->Push(video, audio); }
	}
}

