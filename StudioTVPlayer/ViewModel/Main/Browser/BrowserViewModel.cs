using System.Collections.ObjectModel;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;

namespace StudioTVPlayer.ViewModel.Main.Browser
{
    public class BrowserViewModel : ViewModelBase
    {
        private ObservableCollection<BrowserTabViewModel> _browserTabs = new ObservableCollection<BrowserTabViewModel>();
        public ObservableCollection<BrowserTabViewModel> BrowserTabs
        {
            get => _browserTabs;
            set
            {
                Set(ref _browserTabs, value);
            }
        }

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set
            {
                Set(ref _isFocused, value);
            }
        }


        public BrowserViewModel(IMediaDataProvider mediaDataProvider)
        {
            BrowserTabs = mediaDataProvider.GetBrowserTabs();
        }
    }
}
