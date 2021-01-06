using System;

namespace StudioTVPlayer.Model
{
    public class MediaPlayerProgressEventArgs
    {
        public MediaPlayerProgressEventArgs(TimeSpan time)
        {
            Time = time;
        }

        public TimeSpan Time { get; }
    }
}