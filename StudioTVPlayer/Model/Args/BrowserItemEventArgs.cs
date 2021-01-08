using System;
using StudioTVPlayer.ViewModel.Main.Browser;

namespace StudioTVPlayer.Model.Args
{
    public class BrowserItemEventArgs : EventArgs
    {
        public BrowserMediaViewModel BrowserItem { get; }
        public BrowserItemEventArgs(BrowserMediaViewModel browserItem)
        {
            BrowserItem = browserItem; 
        }
    }
}
