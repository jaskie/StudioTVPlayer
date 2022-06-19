using System;

namespace StudioTVPlayer.Model.Args
{
    public class TimeEventArgs: EventArgs
    {
        public TimeEventArgs(TimeSpan timecode, TimeSpan timeFromBegin, TimeSpan timeToEnd)
        {
            Timecode = timecode;
            TimeFromBegin = timeFromBegin;
            TimeToEnd = timeToEnd;
        }

        public TimeSpan Timecode { get; }
        public TimeSpan TimeFromBegin { get; }
        public TimeSpan TimeToEnd { get; }
    }
}