using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RundownItem
    {
        public RundownItem(Media media)
        {
            Media = media;
        }

        public Media Media { get; }

    }
}
