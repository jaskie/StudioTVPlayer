#include "../pch.h"
#include "Ndi.h"
#include "../Core/OutputDevice.h"
#include "../Core/VideoFormat.h"
#include "../Core/Channel.h"
#include "Processing.NDI.Lib.h"

namespace TVPlayR {
	namespace Ndi {
		
		static NDIlib_v4* LoadNdi()
		{
			HMODULE hNDILib = LoadLibraryA(NDILIB_LIBRARY_NAME);
			if (!hNDILib)
			{
				size_t required_size = 0;
				if (getenv_s(&required_size, NULL, 0, NDILIB_REDIST_FOLDER) != 0 || required_size == 0)
					return nullptr;
				char* p_ndi_runtime_v4 = (char*)malloc(required_size * sizeof(char));
				if (!p_ndi_runtime_v4
					|| getenv_s(&required_size, p_ndi_runtime_v4, required_size, NDILIB_REDIST_FOLDER) != 0
					|| !p_ndi_runtime_v4)
				{
					free(p_ndi_runtime_v4);
					return nullptr;
				}
				std::string ndi_path(p_ndi_runtime_v4);
				free(p_ndi_runtime_v4);
				ndi_path += "\\" NDILIB_LIBRARY_NAME;
				hNDILib = LoadLibraryA(ndi_path.c_str());
				if (!hNDILib)
					return nullptr;
			}
			NDIlib_v4* (*NDIlib_v4_load)(void) = NULL;
			if (hNDILib)
				*((FARPROC*)&NDIlib_v4_load) = GetProcAddress(hNDILib, "NDIlib_v4_load");

			// Unable to load NDI from the library
			if (!NDIlib_v4_load)
			{
				if (hNDILib)
					FreeLibrary(hNDILib);
				return nullptr;
			}
			return NDIlib_v4_load();
		}

		static NDIlib_v4* ndi_library_ = LoadNdi();
		
		struct Ndi::implementation
		{
			Core::Channel * channel_ = nullptr;
			const std::string source_name_;
			const std::string group_name_;
			std::deque<std::shared_ptr<AVFrame>> video_frame_buffer_;
			std::deque<std::shared_ptr<AVFrame>> audio_frame_buffer_;
			const NDIlib_send_instance_t send_instance_;
			
			implementation(const std::string& source_name, const std::string& group_names)
				: send_instance_(CreateSend(source_name, group_names))
			{
			}

			~implementation()
			{
				if (send_instance_)
					ndi_library_->send_destroy(send_instance_);
			}

			NDIlib_send_instance_t CreateSend(const std::string& source_name, const std::string& group_names)
			{
				NDIlib_send_create_t send_create_description;
				send_create_description.p_ndi_name = source_name.c_str();
				send_create_description.p_groups = group_names.c_str();
				return ndi_library_->send_create(&send_create_description);
			}

			bool AssignToChannel(Core::Channel& channel)
			{
				channel_ = &channel;
				return true;
			}

			void ReleaseChannel()
			{
				if (!channel_)
					return;
				channel_ = nullptr;
			}

			bool IsPlaying() const
			{
				return false;
			}
			
			void Push(FFmpeg::AVSync& sync)
			{
				//video_frame_buffer_.emplace_back(sync.Video);
				//audio_frame_buffer_.emplace_back(sync.Audio);
			}
		};
			
		Ndi::Ndi(const std::string& source_name, const std::string& group_name) : impl_(std::make_unique<implementation>(source_name, group_name)) { }
		Ndi::~Ndi() { }

		bool Ndi::AssignToChannel(Core::Channel& channel) { 
			return Core::OutputDevice::AssignToChannel(channel)
				&& impl_->AssignToChannel(channel);
		}

		void Ndi::ReleaseChannel()
		{
			impl_->ReleaseChannel();
		}

		bool Ndi::IsPlaying() const { return impl_->IsPlaying(); }
		void Ndi::Push(FFmpeg::AVSync & sync) { impl_->Push(sync); }
		void Ndi::SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback)
		{
		}
	}
}

