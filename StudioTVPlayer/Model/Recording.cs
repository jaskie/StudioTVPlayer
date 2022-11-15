using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

        public void StartRecording(string fullPath, EncoderPreset preset)
        {
            var decklinkInput = Input as DecklinkInput ?? throw new ArgumentException(nameof(Input));
            FullPath = fullPath;
            EncoderPreset = preset;
            Providers.GlobalApplicationData.Current.AddRecording(this);
            _output = new TVPlayR.FFOutput(address: fullPath, 
                video_codec: preset.VideoCodec, 
                audio_codec: preset.AudioCodec, 
                video_bitrate: preset.VideoBitrate, 
                audio_bitrate: preset.AudioBitrate, 
                options: preset.Options, 
                video_filter: preset.VideoFilter, 
                output_metadata: preset.OutputMetadata, 
                video_metadata: preset.VideoMetadata, 
                audio_metadata: preset.AudioMetadata, 
                video_stream_id: preset.VideoStreamId, 
                audio_stream_id: preset.AudioStreamId);
            _output.Initialize(decklinkInput.CurrentFormat, TVPlayR.PixelFormat.yuv422, 2, 48000);
            Input.AddOutputSink(_output);
        }

        public void StopRecording() 
        {
            Input.RemoveOutputSink(_output);
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
