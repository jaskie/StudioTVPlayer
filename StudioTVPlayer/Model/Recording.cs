using System;

namespace StudioTVPlayer.Model
{
    /// <summary>
    /// describes recording in progress
    /// </summary>
    public class Recording: IDisposable
    {
        private bool _disposed;

        private TVPlayR.FFOutput _output;

        public Recording(InputBase input)
        {
            Input = input;
        }

        public InputBase Input { get; }

        public string FullPath { get; private set; }

        public EncoderPreset EncoderPreset { get; private set; }

        public void StartRecording(string fullPath, TVPlayR.VideoFormat format, EncoderPreset preset)
        {
            FullPath = fullPath;
            EncoderPreset = preset;
            Providers.GlobalApplicationData.Current.AddRecording(this);
            _output = new TVPlayR.FFOutput(address: fullPath, 
                video_codec: preset.VideoCodec, 
                audio_codec: preset.AudioCodec, 
                video_bitrate: preset.VideoBitrate, 
                audio_bitrate: preset.AudioBitrate, 
                options: preset.Options, 
                video_filter: preset.VideoFilter, pixel_format:preset.PixelFormat,
                output_metadata: preset.OutputMetadata, 
                video_metadata: preset.VideoMetadata, 
                audio_metadata: preset.AudioMetadata, 
                video_stream_id: preset.VideoStreamId, 
                audio_stream_id: preset.AudioStreamId,
                preset.OutputFormat
                );
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.InputInitialized += DecklinkInput_InputInitialized;
            }
            _output.Initialize(format, TVPlayR.PixelFormat.yuv422, 2, 48000);
            Input.AddOutputSink(_output);
        }

        private void DecklinkInput_InputInitialized(object sender, EventArgs e)
        {
            StopRecording();
        }

        public void StopRecording() 
        {
            Input.RemoveOutputSink(_output);
            if (Input is DecklinkInput decklinkInput)
            {
                decklinkInput.InputInitialized -= DecklinkInput_InputInitialized;
            }
            _output.Dispose();
            _output = null;
            Finished?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Finished;

        public void Dispose()
        {
            if (!_disposed)
            {
                StopRecording();
                _disposed = true;
            }
        }
    }
}
