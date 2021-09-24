using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System.Windows.Input;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class FileRundownItemViewModel : ViewModelBase
    {
        private bool _isLoaded;

        public FileRundownItemViewModel(FileRundownItem rundownItem)
        {
            RundownItem = rundownItem;
            CommandRemove = new UiCommand(Remove, _ => !IsLoaded);
        }

        public bool IsLoaded
        {
            get => _isLoaded;
            set
            {
                if (!Set(ref _isLoaded, value))
                    return;
                Refresh();
            }
        }

        public ICommand CommandRemove { get; }

        public FileRundownItem RundownItem { get; }

        public ImageSource Thumbnail => RundownItem.Media.Thumbnail;

        private void Remove(object _)
        {
            RundownItem.RemoveFromRundown();
        }

    }
}
