using StudioTVPlayer.Model;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class FileRundownItemViewModel(FileRundownItem rundownItem) : RundownItemViewModelBase
    {
        public override RundownItemBase RundownItem => rundownItem;
    }
}
