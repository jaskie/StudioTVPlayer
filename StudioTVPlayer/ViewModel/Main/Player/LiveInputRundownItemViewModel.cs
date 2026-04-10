using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class LiveInputRundownItemViewModel(LiveInputRundownItem liveInputRundownItem) : RundownItemViewModelBase
    {
        public override RundownItemBase RundownItem => liveInputRundownItem;
    }
}
