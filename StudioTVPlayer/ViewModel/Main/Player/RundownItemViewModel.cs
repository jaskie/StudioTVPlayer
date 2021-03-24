using StudioTVPlayer.Model;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class RundownItemViewModel : ViewModelBase
    {
        private bool _isLoaded;
        private bool _isDisabled;

        public RundownItemViewModel(RundownItem rundownItem)
        {
            RundownItem = rundownItem;
        }

        public bool IsLoaded
        {
            get => _isLoaded;
            set => Set(ref _isLoaded, value);
        }

        public bool IsDisabled
        {
            get => _isDisabled;
            set => Set(ref _isDisabled, value);
        }

        public RundownItem RundownItem { get; }

        public ImageSource Thumbnail => RundownItem.Media.Thumbnail;

    }
}
