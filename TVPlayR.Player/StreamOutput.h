#pragma once
#include "OutputBase.h"
#include "FFMpeg/FFStreamOutput.h"

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
			String^ audio_metadata,
			String^ video_metadata,
			String^ options,
			int video_stream_id,
			int audio_stream_id
			);
		~StreamOutput();
		!StreamOutput();
	private:
		std::shared_ptr<FFmpeg::FFStreamOutput>* _native_output;
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_native_output; }
	};

}
