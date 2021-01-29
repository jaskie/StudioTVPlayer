using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Args
{
    public class RundownItemEventArgs : EventArgs
    {
        public RundownItemEventArgs(RundownItem mediaPlayerQueueItem)
        {
            MediaPlayerQueueItem = mediaPlayerQueueItem;
        }

        public RundownItem MediaPlayerQueueItem { get; }
    }
}
