using System.Collections.ObjectModel;

namespace StudioTVPlayer.ViewModel.Main.Browser
{
    public class BrowsersViewModel : ViewModelBase
    {
        private ObservableCollection<BrowserViewModel> _browserTabs = new ObservableCollection<BrowserViewModel>();
        public ObservableCollection<BrowserViewModel> BrowserTabs
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
    }
}
