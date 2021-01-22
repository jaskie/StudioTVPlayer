using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Args
{
    public class MediaEventArgs : EventArgs
    {
        public Media Media { get; }
        public MediaEventArgs(Media m)
        {
            Media = m;
        }
    }
}
