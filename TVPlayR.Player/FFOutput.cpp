#include "stdafx.h"
#include "FFOutput.h"
#include "ClrStringHelper.h"
#include "OverlayBase.h"
#include "FFmpeg/FFmpegOutput.h"
#include "FFmpeg/FFOutputParams.h"
#include "Player.h"
#include "Core/Player.h"
#include "Core/VideoFormat.h"

namespace TVPlayR
{
    FFmpeg::FFmpegOutput* CreateNativeFFOutput(
        String^ url,
        String^ video_codec, String^ audio_codec,
        int video_bitrate, int audio_bitrate,
        String^ options,
        String^ video_filter,
        String^ output_metadata, String^ video_metadata, String^ audio_metadata,
        int video_stream_id, int audio_stream_id
    )
    {
        REWRAP_EXCEPTION(return new FFmpeg::FFmpegOutput(FFmpeg::FFOutputParams{
            ClrStringToStdString(url),
            ClrStringToStdString(video_codec), ClrStringToStdString(audio_codec),
            video_bitrate, audio_bitrate,
            ClrStringToStdString(options),
            ClrStringToStdString(video_filter),
            ClrStringToStdString(output_metadata), ClrStringToStdString(video_metadata), ClrStringToStdString(audio_metadata),
            video_stream_id, audio_stream_id
            });)
    }

    FFOutput::FFOutput(
        String^ url, 
        String^ video_codec, String^ audio_codec, 
        int video_bitrate, int audio_bitrate, 
        String^ options,
        String^ video_filter,
        String^ output_metadata, String^ video_metadata, String^ audio_metadata, 
        int video_stream_id, int audio_stream_id
    )
        : _native_output(new std::shared_ptr<FFmpeg::FFmpegOutput>(CreateNativeFFOutput(url, video_codec, audio_codec, video_bitrate, audio_bitrate, options, video_filter, output_metadata, video_metadata, audio_metadata, video_stream_id, audio_stream_id)))
    { }

    FFOutput::~FFOutput()
    {
        this->!FFOutput();
    }

    FFOutput::!FFOutput()
    {
        if (!_native_output)
            return;
        REWRAP_EXCEPTION(
            (*_native_output)->Uninitialize();
            delete _native_output;)
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

    void FFOutput::InitializeFor(Player^ player)
    {
        REWRAP_EXCEPTION(
            Core::Player& native_player = player->GetNativePlayer();
            (*_native_output)->Initialize(native_player.Format().type(), native_player.PixelFormat(), native_player.AudioChannelsCount(), native_player.AudioSampleRate());)
    }

    std::shared_ptr<Core::OutputDevice> FFOutput::GetNativeDevice() { return _native_output ? *_native_output : nullptr; }

    std::shared_ptr<Core::OutputSink> FFOutput::GetNativeSink() { return _native_output ? *_native_output : nullptr; }
    
}
