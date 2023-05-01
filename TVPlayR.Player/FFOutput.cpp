#include "stdafx.h"
#include "FFOutput.h"
#include "ClrStringHelper.h"
#include "OverlayBase.h"
#include "FFmpeg/FFmpegOutput.h"
#include "FFmpeg/FFOutputParams.h"
#include "VideoFormat.h"
#include "Core/VideoFormat.h"

namespace TVPlayR
{
    FFmpeg::FFmpegOutput* CreateNativeFFOutput(
        String^ url,
        String^ video_codec, String^ audio_codec,
        int video_bitrate, int audio_bitrate,
        String^ options,
        String^ video_filter, String^ pixel_format,
        String^ output_metadata, String^ video_metadata, String^ audio_metadata,
        int video_stream_id, int audio_stream_id,
        String^ output_format
    )
    {
        REWRAP_EXCEPTION(return new FFmpeg::FFmpegOutput(FFmpeg::FFOutputParams{
            ClrStringToStdString(url),
            ClrStringToStdString(video_codec), ClrStringToStdString(audio_codec),
            video_bitrate, audio_bitrate,
            ClrStringToStdString(options),
            ClrStringToStdString(video_filter), ClrStringToStdString(pixel_format),
            ClrStringToStdString(output_metadata), ClrStringToStdString(video_metadata), ClrStringToStdString(audio_metadata),
            video_stream_id, audio_stream_id,
            ClrStringToStdString(output_format)
            });)
    }

    FFOutput::FFOutput(
        String^ url, 
        String^ video_codec, String^ audio_codec, 
        int video_bitrate, int audio_bitrate, 
        String^ options,
        String^ video_filter, String^ pixel_format,
        String^ output_metadata, String^ video_metadata, String^ audio_metadata, 
        int video_stream_id, int audio_stream_id,
        String^ output_format
    )
        : _native_output(new std::shared_ptr<FFmpeg::FFmpegOutput>(CreateNativeFFOutput(url, video_codec, audio_codec, video_bitrate, audio_bitrate, options, video_filter, pixel_format, output_metadata, video_metadata, audio_metadata, video_stream_id, audio_stream_id, output_format)))
    { }

    FFOutput::~FFOutput()
    {
        this->!FFOutput();
    }

    FFOutput::!FFOutput()
    {
        if (!_native_output)
            return;
        REWRAP_EXCEPTION(delete _native_output;)
        _native_output = nullptr;
    }

    void FFOutput::AddOverlay(OverlayBase^ overlay)
    {
        if (!_native_output)
            return;
        REWRAP_EXCEPTION((*_native_output)->AddOverlay(overlay->GetNativeObject());)
    }

    void FFOutput::RemoveOverlay(OverlayBase^ overlay)
    {
        if (!_native_output)
            return;
        REWRAP_EXCEPTION((*_native_output)->RemoveOverlay(overlay->GetNativeObject());)
    }

    void FFOutput::Initialize(VideoFormat^ format, PixelFormat pixelFormat, int audioChannelsCount, int audioSampleRate)
    {
        REWRAP_EXCEPTION((*_native_output)->Initialize(format->GetNativeEnumType(), pixelFormat, audioChannelsCount, audioSampleRate);)
    }

    std::shared_ptr<Core::OutputDevice> FFOutput::GetNativeDevice() { return _native_output ? *_native_output : nullptr; }

    std::shared_ptr<Core::OutputSink> FFOutput::GetNativeSink() { return _native_output ? *_native_output : nullptr; }
    
}
