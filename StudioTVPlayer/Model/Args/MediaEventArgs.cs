using System;

namespace StudioTVPlayer.Model.Args
{
    public class MediaEventArgs(MediaFile m, MediaEventKind kind) : EventArgs
    {
        public MediaFile Media { get; } = m;
        public MediaEventKind Kind { get; } = kind;
    }
}
