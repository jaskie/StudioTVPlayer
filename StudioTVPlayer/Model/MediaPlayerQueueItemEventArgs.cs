using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class MediaPlayerQueueItemEventArgs : EventArgs
    {
        public MediaPlayerQueueItemEventArgs(MediaPlayerQueueItem mediaPlayerQueueItem)
        {
            MediaPlayerQueueItem = mediaPlayerQueueItem;
        }

        public MediaPlayerQueueItem MediaPlayerQueueItem { get; }
    }
}
