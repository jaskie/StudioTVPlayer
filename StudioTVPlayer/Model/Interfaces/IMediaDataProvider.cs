using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using StudioTVPlayer.Services;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IMediaDataProvider
    {        
        List<BrowserTabItemViewModel> GetBrowserTabItems(string path);
        ObservableCollection<BrowserTabViewModel> GetBrowserTabs();
        BrowserTabItemViewModel GetNewBrowserTabItem(Media m);
        MediaWatcherService StartWatcher(string path);
        void LoadMediaFiles(ICollectionView collection, IMediaWatcherService mediaWatcher);
        void UnloadMediaFiles(ICollectionView collection);
    }
}
