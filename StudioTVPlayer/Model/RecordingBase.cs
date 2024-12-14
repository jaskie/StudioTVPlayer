using System;

namespace StudioTVPlayer.Model
{
    public abstract class RecordingBase
    {
        private bool _disposed;

        private TVPlayR.FFOutput _output;

        public RecordingBase(InputBase input)
        {
            Input = input;
        }

        public EncoderPreset EncoderPreset { get; private set; }

        public string FullPath { get; private set; }

        public InputBase Input { get; }

        public event EventHandler Finished;

        public void Dispose()
        {
            if (!_disposed)
            {
                StopRecording();
                _disposed = true;
            }
        }

        public virtual void StartRecording(string fullPath, TVPlayR.VideoFormat format, EncoderPreset preset)
        {
            FullPath = fullPath;
            EncoderPreset = preset;
            _output = new TVPlayR.FFOutput(
                address: fullPath,
                video_codec: preset.VideoCodec,
                audio_codec: preset.AudioCodec,
                video_bitrate: preset.VideoBitrate,
                audio_bitrate: preset.AudioBitrate,
                options: preset.Options,
                video_filter: preset.VideoFilter,
                pixel_format: preset.PixelFormat,
                output_metadata: preset.OutputMetadata,
                video_metadata: preset.VideoMetadata,
                audio_metadata: preset.AudioMetadata,
                video_stream_id: preset.VideoStreamId,
                audio_stream_id: preset.AudioStreamId,
                output_format: preset.OutputFormat
                );
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.FormatChanged += DecklinkInput_FormatChanged;
            }
            _output.Initialize(format, TVPlayR.PixelFormat.yuv422, 2, 48000);
            Input.AddOutputSink(_output);
        }

        public void StopRecording()
        {
            Input.RemoveOutputSink(_output);
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.FormatChanged -= DecklinkInput_FormatChanged;
            }
            _output.Dispose();
            _output = null;
            Finished?.Invoke(this, EventArgs.Empty);
        }

        private void DecklinkInput_FormatChanged(object sender, EventArgs e)
        {
            StopRecording();
        }
    }
}