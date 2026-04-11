using System;

namespace StudioTVPlayer.Model.Args
{
    public class AudioVolumeEventArgs(float[] audioVolume) : EventArgs
    {
        public float[] AudioVolume { get; } = audioVolume;
    }
}