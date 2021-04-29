using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System;
using System.Windows.Input;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Main.Player
{
    public class RundownItemViewModel : ViewModelBase
    {
        private bool _isLoaded;
        private bool _isDisabled;
        private bool _isAutoStart;
        private bool _isLoop;

        public RundownItemViewModel(RundownItem rundownItem)
        {
            RundownItem = rundownItem;
            IsDisabled = !rundownItem.Enabled;
            IsAutoStart = rundownItem.IsAutoStart;
            IsLoop = rundownItem.IsLoop;
            CommandRemove = new UiCommand(Remove, _ => !IsLoaded);
        }

        public bool IsLoaded
        {
            get => _isLoaded;
            set => Set(ref _isLoaded, value);
        }

        public bool IsDisabled
        {
            get => _isDisabled;
            set
            {
                if (!Set(ref _isDisabled, value))
                    return;
                RundownItem.Enabled = !value;
            }
        }

        public bool IsAutoStart
        {
            get => _isAutoStart;
            set
            {
                if (!Set(ref _isAutoStart, value))
                    return;
                RundownItem.IsAutoStart = value;
            }
        }

        public bool IsLoop
        {
            get => _isLoop; 
            set
            {
                if (!Set(ref _isLoop, value))
                    return;
                RundownItem.IsLoop = value;
            }
        }

        public ICommand CommandRemove { get; }

        public RundownItem RundownItem { get; }

        public ImageSource Thumbnail => RundownItem.Media.Thumbnail;

        private void Remove(object _)
        {
            RundownItem.RemoveFromRundown();
        }

    }
}
