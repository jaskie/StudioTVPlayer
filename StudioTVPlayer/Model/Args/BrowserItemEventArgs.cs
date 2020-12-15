using System;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;

namespace StudioTVPlayer.Model.Args
{
    public class BrowserItemEventArgs : EventArgs
    {
        public BrowserTabItemViewModel BrowserItem { get; }
        public BrowserItemEventArgs(BrowserTabItemViewModel browserItem)
        {
            BrowserItem = browserItem; 
        }
    }
}
