using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using StudioTVPlayer.Model.Args;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IMediaWatcherService
    {
        event EventHandler<MediaEventArgs> NotifyOnMediaChanged;
        event EventHandler<MediaEventArgs> NotifyOnMediaAdd;
        event EventHandler<MediaEventArgs> NotifyOnMediaVerified;
        event EventHandler<MediaEventArgs> NotifyOnMediaDelete;
        event EventHandler<MediaEventArgs> NotifyOnMediaRenamed;

        void AddMediaToTrack(Media m);
        void AddMediaToVerify(Media m);

        string GetPath();
    }
}
