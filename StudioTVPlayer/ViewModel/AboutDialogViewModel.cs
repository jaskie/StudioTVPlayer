using StudioTVPlayer.Providers;
using System;

namespace StudioTVPlayer.ViewModel
{
    public class AboutDialogViewModel: DialogViewModelBase
    {
        public AboutDialogViewModel(Action<DialogViewModelBase> closeHandler) : base(closeHandler)
        {
        }

        public VersionInfo VersionInfo => VersionInfo.Current;
    }
}
