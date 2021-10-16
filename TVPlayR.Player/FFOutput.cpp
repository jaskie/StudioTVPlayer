#include "stdafx.h"
#include "FFOutput.h"
#include "ClrStringHelper.h"
#include "FFmpeg/FFOutputParams.h"

namespace TVPlayR
{

    FFOutput::FFOutput(String^ address, String^ video_codec, String^ audio_codec, int video_bitrate, int audio_bitrate, String^ output_filter, String^ output_metadata, String^ video_metadata, String^ audio_metadata, String^ options, int video_stream_id, int audio_stream_id)
        : _native_output(new std::shared_ptr<FFmpeg::FFmpegOutput>(std::make_shared<FFmpeg::FFmpegOutput>(FFmpeg::FFOutputParams{
            ClrStringToStdString(address),
            ClrStringToStdString(video_codec),
            ClrStringToStdString(audio_codec),
            video_bitrate,
            audio_bitrate,
            ClrStringToStdString(output_filter),
            ClrStringToStdString(output_metadata),
            ClrStringToStdString(video_metadata),
            ClrStringToStdString(audio_metadata),
            ClrStringToStdString(options),
            video_stream_id,
            audio_stream_id
            })))
    { }

    FFOutput::~FFOutput()
    {
        this->!FFOutput();
    }

    FFOutput::!FFOutput()
    {
        if (!_native_output)
            return;
        delete _native_output;
        _native_output = nullptr;
    }
}
