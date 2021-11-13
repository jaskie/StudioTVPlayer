#pragma once
#include "Processing.NDI.Lib.h"

namespace TVPlayR {
	namespace Core {
		class VideoFormat;
	}
	namespace Ndi {
		NDIlib_v4* LoadNdi();
		void UnloadNdi();
		const NDIlib_send_instance_t CreateSend(NDIlib_v4* const ndi, const std::string& source_name, const std::string& group_names);
		NDIlib_video_frame_v2_t CreateVideoFrame(const Core::VideoFormat& format, const std::shared_ptr<AVFrame>& avframe, int64_t timecode);
		NDIlib_audio_frame_interleaved_32s_t CreateAudioFrame(std::shared_ptr<AVFrame> avframe, int64_t timecode);
}}