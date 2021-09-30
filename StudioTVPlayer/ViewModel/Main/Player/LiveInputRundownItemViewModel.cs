using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class LiveInputRundownItemViewModel : RundownItemViewModelBase
    {
        private readonly LiveInputRundownItem _liveInputRundownItem;

        public LiveInputRundownItemViewModel(LiveInputRundownItem liveInputRundownItem)
        {
            _liveInputRundownItem = liveInputRundownItem;
        }
        public override RundownItemBase RundownItem => _liveInputRundownItem;
    }
}
