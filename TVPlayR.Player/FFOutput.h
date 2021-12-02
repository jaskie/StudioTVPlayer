#pragma once
#include "OutputBase.h"

using namespace System;

namespace TVPlayR {

	namespace Core {
		class OutputDevice;
	}
	namespace FFmpeg {
		class FFmpegOutput;
	}

	public ref class FFOutput sealed : public OutputBase
	{
	public:
		FFOutput(
			String^ address, 
			String^ video_codec, 
			String^ audio_codec,
			int video_bitrate,
			int audio_bitrate,
			String^ options,
			String^ video_filter,
			String^ output_metadata,
			String^ video_metadata,
			String^ audio_metadata,
			int video_stream_id,
			int audio_stream_id
			);
		~FFOutput();
		!FFOutput();
		property static array<String^>^ VideoCodecs { array<String^>^ get() { return _videoCodecs; }};
		property static array<String^>^ AudioCodecs { array<String^>^ get() { return _audioCodecs; }};
	private:
		std::shared_ptr<FFmpeg::FFmpegOutput>* _native_output;
		static array<String^>^ _videoCodecs = gcnew array<String^> { "mpeg2video", "libx264", "h264_nvenc",	"hevc_nvenc"};
		static array<String^>^ _audioCodecs = gcnew array<String^> { "aac", "ac3", "libmp3lame", "mp2" };
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override;
	};

}
