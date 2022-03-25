using System;

namespace StudioTVPlayer.Model.Args
{
    public class AudioVolumeEventArgs : EventArgs
    {
        public AudioVolumeEventArgs(float[] audioVolume)
        {
            AudioVolume = audioVolume;
        }

        public float[] AudioVolume { get; }
    }
}