using System;

namespace StudioTVPlayer.Model.Args
{
    public class TimeEventArgs
    {
        public TimeEventArgs(TimeSpan time)
        {
            Time = time;
        }

        public TimeSpan Time { get; }
    }
}