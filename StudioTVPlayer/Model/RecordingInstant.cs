using System;

namespace StudioTVPlayer.Model
{
    /// <summary>
    /// describes instant recording
    /// </summary>
    public sealed class RecordingInstant: RecordingBase, IDisposable
    {
        public RecordingInstant(InputBase input) : base(input) { }

        public override void StartRecording(string fullPath, TVPlayR.VideoFormat format, EncoderPreset preset)
        {
            Providers.GlobalApplicationData.Current.AddRecording(this);
            base.StartRecording(fullPath, format, preset);
        }
    }
}
