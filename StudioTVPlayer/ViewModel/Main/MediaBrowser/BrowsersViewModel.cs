using StudioTVPlayer.Providers;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Main.MediaBrowser
{
    public class BrowsersViewModel : ViewModelBase
    {
        public BrowsersViewModel()
        {
            Browsers = GlobalApplicationData.Current.Configuration.WatchedFolders.Select(f => new BrowserViewModel(f)).ToArray();
        }

        public BrowserViewModel[] Browsers { get; }
    }
}
