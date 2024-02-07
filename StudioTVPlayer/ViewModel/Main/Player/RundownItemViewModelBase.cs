using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System.Windows.Input;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public abstract class RundownItemViewModelBase: ViewModelBase
    {
        protected RundownItemViewModelBase()
        {
            RemoveCommand = new UiCommand(Remove, _ => !IsLoaded);
        }

        private bool _isLoaded;

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

        public ICommand RemoveCommand { get; }

        public abstract RundownItemBase RundownItem { get; }

        public ImageSource Thumbnail => RundownItem.Thumbnail;

        private void Remove(object _)
        {
            RundownItem.RemoveFromRundown();
        }
    }
}