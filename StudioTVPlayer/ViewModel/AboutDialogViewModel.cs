using StudioTVPlayer.Providers;
using System;

namespace StudioTVPlayer.ViewModel
{
    public class AboutDialogViewModel(Action<DialogViewModelBase> closeHandler) : DialogViewModelBase(closeHandler)
    {
        public VersionInfo VersionInfo => VersionInfo.Current;
    }
}
