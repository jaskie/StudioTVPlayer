using StudioTVPlayer.Helpers;
using System;
using System.Diagnostics;

namespace StudioTVPlayer.ViewModel
{
    public abstract class DialogViewModelBase : ViewModelBase
    {

        protected DialogViewModelBase(Action<DialogViewModelBase> closeHandler)
        {
            CloseCommand = new UiCommand(_ => closeHandler(this));
            OpenHyperlinkCommand = new UiCommand(param => Process.Start(param as string));
        }

        public UiCommand CloseCommand { get; }

        public UiCommand OpenHyperlinkCommand { get; }
    }
}