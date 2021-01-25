using StudioTVPlayer.Providers;
using System;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Main.MediaBrowser
{
    public class BrowsersViewModel : ViewModelBase, IDisposable
    {
        public BrowsersViewModel()
        {
            Browsers = GlobalApplicationData.Current.Configuration.WatchedFolders.Select(f => new BrowserViewModel(f)).ToArray();
            SelectedBrowser = Browsers.FirstOrDefault();
        }

        public BrowserViewModel[] Browsers { get; }

        public BrowserViewModel SelectedBrowser { get; set; }

        public void Dispose()
        {
            foreach (var browser in Browsers)
                browser.Dispose();
        }
    }
}
