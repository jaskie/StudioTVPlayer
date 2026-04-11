using StudioTVPlayer.Helpers;
using System;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel
{
    public abstract class RemovableViewModelBase: ModifyableViewModelBase
    {
        public event EventHandler RemoveRequested;

        protected RemovableViewModelBase()
        {
            RequestRemoveCommand = new UiCommand(RequestRemove, CanRequestRemove);
        }

        public ICommand RequestRemoveCommand { get; }

        protected virtual bool CanRequestRemove(object obj)
        {
            return true;
        }

        protected virtual void RequestRemove(object obj)
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }
    }
}
