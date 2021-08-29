#pragma once
#include "OutputBase.h"
#include "FFMpeg/FFStreamOutput.h"

using namespace System;

namespace TVPlayR {

	public ref class StreamOutput sealed : public OutputBase
	{
	public:
		StreamOutput(
			String^ address, 
			String^ video_codec, 
			String^ audio_codec,
			int video_bitrate,
			int audio_bitrate,
			String^ output_filter,
			String^ output_metadata,
			String^ video_metadata,
			String^ audio_metadata,
			String^ options,
			int video_stream_id,
			int audio_stream_id
			);
		~StreamOutput();
		!StreamOutput();
		property static array<String^>^ VideoCodecs { array<String^>^ get() { return _videoCodecs; }};
		property static array<String^>^ AudioCodecs { array<String^>^ get() { return _audioCodecs; }};
	private:
		std::shared_ptr<FFmpeg::FFStreamOutput>* _native_output;
		static array<String^>^ _videoCodecs = gcnew array<String^> { "mpeg2video", "libx264", "h264_nvenc",	"hevc_nvenc"};
		static array<String^>^ _audioCodecs = gcnew array<String^> { "aac", "ac3", "libmp3lame", "mp2" };
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_native_output; }
	};

}
