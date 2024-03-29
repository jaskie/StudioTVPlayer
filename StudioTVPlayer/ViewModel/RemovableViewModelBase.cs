﻿using StudioTVPlayer.Helpers;
using System;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel
{
    public abstract class RemovableViewModelBase: ModifyableViewModelBase
    {
        public event EventHandler RemoveRequested;

        protected RemovableViewModelBase()
        {
            RemoveCommand = new UiCommand(Remove, CanRequestRemove);
        }

        public ICommand RemoveCommand { get; }

        protected virtual bool CanRequestRemove(object obj)
        {
            return true;
        }

        protected virtual void Remove(object obj)
        {
            RemoveRequested?.Invoke(this, EventArgs.Empty);
        }
    }
}
