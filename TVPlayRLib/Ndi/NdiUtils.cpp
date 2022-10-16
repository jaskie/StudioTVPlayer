#include "../pch.h"
#include "NdiUtils.h"
#include "../Core/VideoFormat.h"
#include "../FieldOrder.h"

namespace TVPlayR {
	namespace Ndi {

		static NDIlib_v4* ndi_library = nullptr;
		HMODULE hNDILib = nullptr;

		NDIlib_v4* LoadNdi()
		{
			if (ndi_library)
				return ndi_library;
			hNDILib = LoadLibraryA(NDILIB_LIBRARY_NAME);
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
			ndi_library = NDIlib_v4_load();
			return ndi_library;
		}

		void UnloadNdi()
		{
			ndi_library = nullptr;
			if (hNDILib)
				FreeLibrary(hNDILib);
			hNDILib = nullptr;
		}

		const NDIlib_send_instance_t CreateSend(NDIlib_v4* const ndi, const std::string& source_name, const std::string& group_names)
		{
			assert(ndi);
			NDIlib_send_create_t send_create_description;
			send_create_description.p_ndi_name = source_name.c_str();
			send_create_description.p_groups = group_names.empty() ? NULL : group_names.c_str();
			send_create_description.clock_audio = false;
			send_create_description.clock_video = true;
			return ndi->send_create(&send_create_description);
		}

		NDIlib_video_frame_v2_t CreateVideoFrame(const Core::VideoFormat& format, const std::shared_ptr<AVFrame>& avframe, std::int64_t timecode)
		{
			if (!avframe)
				THROW_EXCEPTION("CreateVideoFrame: no frame provided");
			if (format.type() == Core::VideoFormatType::invalid)
				THROW_EXCEPTION("CreateVideoFrame: invalid video format");
			NDIlib_FourCC_video_type_e fourcc;
			switch (avframe->format)
			{
			case AV_PIX_FMT_BGRA:
				fourcc = NDIlib_FourCC_type_BGRA;
				break;
			case AV_PIX_FMT_UYVY422:
				fourcc = NDIlib_FourCC_type_UYVY;
				break;
			default:
				THROW_EXCEPTION("CreateVideoFrame: Invalid format of video frame");
			}
			NDIlib_frame_format_type_e frame_format_type;
			switch (format.field_order())
			{
			case TVPlayR::FieldOrder::BottomFieldFirst:
			case TVPlayR::FieldOrder::TopFieldFirst:
				frame_format_type = NDIlib_frame_format_type_interleaved;
				break;
			default:
				frame_format_type = NDIlib_frame_format_type_progressive;
				break;
			}
			return NDIlib_video_frame_v2_t(
				avframe->width,
				avframe->height,
				fourcc,
				format.FrameRate().Numerator(),
				format.FrameRate().Denominator(),
				static_cast<float>(format.SampleAspectRatio().Numerator() * format.width()) / static_cast<float>(format.SampleAspectRatio().Denominator() * format.height()),				frame_format_type,
				timecode * 10,
				avframe->data[0],
				avframe->linesize[0]
			);
		}

		NDIlib_audio_frame_interleaved_32f_t CreateAudioFrame(std::shared_ptr<AVFrame> avframe, std::int64_t timecode)
		{
			assert(avframe->format == AV_SAMPLE_FMT_FLT);
			return NDIlib_audio_frame_interleaved_32f_t(
				avframe->sample_rate,
				avframe->ch_layout.nb_channels,
				avframe->nb_samples,
				timecode * 10,
				reinterpret_cast<float*>(avframe->data[0])
			);
		}


}}