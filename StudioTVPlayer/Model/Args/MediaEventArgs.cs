using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
