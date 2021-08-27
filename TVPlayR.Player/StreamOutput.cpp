#include "stdafx.h"
#include "StreamOutput.h"
#include "ClrStringHelper.h"
#include "FFMpeg/FFStreamOutputParams.h"

namespace TVPlayR
{

    StreamOutput::StreamOutput(String^ address, String^ video_codec, String^ audio_codec, int video_bitrate, int audio_bitrate, String^ output_filter, String^ output_metadata, String^ audio_metadata, String^ video_metadata, String^ options, int video_stream_id, int audio_stream_id)
    {
        const FFmpeg::FFStreamOutputParams params {
            ClrStringToStdString(address), 
            ClrStringToStdString(video_codec),
            ClrStringToStdString(audio_codec),
            video_bitrate,
            audio_bitrate,
            ClrStringToStdString(output_filter),
            ClrStringToStdString(output_metadata),
            ClrStringToStdString(audio_metadata),
            ClrStringToStdString(video_metadata),
            ClrStringToStdString(options),
            video_stream_id,
            audio_stream_id
        };
        _native_output = new std::shared_ptr<FFmpeg::FFStreamOutput>(std::make_shared<FFmpeg::FFStreamOutput>(params));
    }
    StreamOutput::~StreamOutput()
    {
        this->!StreamOutput();
    }
    StreamOutput::!StreamOutput()
    {
        if (!_native_output)
            return;
        delete _native_output;
        _native_output = nullptr;

    }
}
