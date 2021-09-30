using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System.Windows.Input;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class FileRundownItemViewModel : RundownItemViewModelBase
    {
        private readonly FileRundownItem _rundownItem;

        public FileRundownItemViewModel(FileRundownItem rundownItem)
        {
            _rundownItem = rundownItem;
        }

        public override RundownItemBase RundownItem => _rundownItem;
    }
}
