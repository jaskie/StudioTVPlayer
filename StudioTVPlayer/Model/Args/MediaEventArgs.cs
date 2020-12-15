using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Args
{
    public class MediaEventArgs : EventArgs
    {
        public string OldPath { get; }
        public Media Media { get; }
        public MediaEventArgs(Media m, string oldPath=null)
        {
            Media = m;
            OldPath = oldPath;
        }
    }
}
