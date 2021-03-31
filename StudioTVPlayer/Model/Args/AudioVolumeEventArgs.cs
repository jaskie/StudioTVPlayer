using System;

namespace StudioTVPlayer.Model.Args
{
    public class AudioVolumeEventArgs : EventArgs
    {
        public AudioVolumeEventArgs(double audioVolume)
        {
            AudioVolume = audioVolume;
        }

        public double AudioVolume { get; }
    }
}