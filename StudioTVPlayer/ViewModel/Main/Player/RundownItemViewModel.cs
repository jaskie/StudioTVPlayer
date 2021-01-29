using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class RundownItemViewModel : ViewModelBase
    {

        public RundownItemViewModel(RundownItem rundownItem)
        {
            RundownItem = rundownItem;
        }

        private bool _isLoaded;
        public bool IsLoaded
        {
            get => _isLoaded;
            set => Set(ref _isLoaded, value);
        }

        private bool _isDisabled;
        public bool IsDisabled
        {
            get => _isDisabled;
            set => Set(ref _isDisabled, value);
        }

        public RundownItem RundownItem { get; }

    }
}
