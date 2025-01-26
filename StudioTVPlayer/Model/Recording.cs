using System;

namespace StudioTVPlayer.Model
{
    public sealed class Recording : IDisposable
    {
        private object _stopLock = new object();
        private bool _disposed;
        private TVPlayR.FFOutput _outputFile;

        public Recording(InputBase input, EncoderPreset encoderPreset, string fullPath)
        {
            Input = input;
            EncoderPreset = encoderPreset;
            FullPath = fullPath;
        }

        public EncoderPreset EncoderPreset { get; }

        public string FullPath { get; }

        public InputBase Input { get; }

        public event EventHandler Finished;

        public void Dispose()
        {
            if (!_disposed)
            {
                Stop();
                _disposed = true;
            }
        }

        public void Start()
        {
            _outputFile = new TVPlayR.FFOutput(
                address: FullPath,
                video_codec: EncoderPreset.VideoCodec,
                audio_codec: EncoderPreset.AudioCodec,
                video_bitrate: EncoderPreset.VideoBitrate,
                audio_bitrate: EncoderPreset.AudioBitrate,
                options: EncoderPreset.Options,
                video_filter: EncoderPreset.VideoFilter,
                pixel_format: EncoderPreset.PixelFormat,
                output_metadata: EncoderPreset.OutputMetadata,
                video_metadata: EncoderPreset.VideoMetadata,
                audio_metadata: EncoderPreset.AudioMetadata,
                video_stream_id: EncoderPreset.VideoStreamId,
                audio_stream_id: EncoderPreset.AudioStreamId,
                output_format: EncoderPreset.OutputFormat
                );
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.FormatChanged += DecklinkInput_FormatChanged;
            }
            _outputFile.Initialize(Input.CurrentFormat(), TVPlayR.PixelFormat.yuv422, 2, 48000);
            Input.AddOutputSink(_outputFile);
            Providers.GlobalApplicationData.Current.AddRecording(this);
        }

        public void Stop()
        {
            lock (_stopLock)
            {
                if (_outputFile is null)
                {
                    return;
                }
                Input.RemoveOutputSink(_outputFile);
                if (Input is DecklinkInput decklinkInput)
                {
                    decklinkInput.FormatChanged -= DecklinkInput_FormatChanged;
                }
                _outputFile.Dispose();
                _outputFile = null;
                Finished?.Invoke(this, EventArgs.Empty);
            }
        }

        private void DecklinkInput_FormatChanged(object sender, EventArgs e)
        {
            Stop(); // can't continue recording with a different format
        }
    }
}