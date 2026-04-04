using System;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.Model.Args
{
    public class BrowserItemEventArgs(MediaViewModel browserItem) : EventArgs
    {
        public MediaViewModel BrowserItem { get; } = browserItem;
    }
}
