using System;

namespace StudioTVPlayer.Model.Args
{
    public class TimeEventArgs: EventArgs
    {
        public TimeEventArgs(TimeSpan time)
        {
            Time = time;
        }

        public TimeSpan Time { get; }
    }
}