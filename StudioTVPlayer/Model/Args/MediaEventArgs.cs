using System;

namespace StudioTVPlayer.Model.Args
{
    public class MediaEventArgs : EventArgs
    {
        public MediaFile Media { get; }
        public MediaEventKind Kind { get; }

        public MediaEventArgs(MediaFile m, MediaEventKind kind)
        {
            Media = m;
            Kind = kind;
        }

    }
}
