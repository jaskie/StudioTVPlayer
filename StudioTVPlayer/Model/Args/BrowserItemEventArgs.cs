using System;
using StudioTVPlayer.ViewModel.Main.MediaBrowser;

namespace StudioTVPlayer.Model.Args
{
    public class BrowserItemEventArgs : EventArgs
    {
        public MediaViewModel BrowserItem { get; }
        public BrowserItemEventArgs(MediaViewModel browserItem)
        {
            BrowserItem = browserItem; 
        }
    }
}
