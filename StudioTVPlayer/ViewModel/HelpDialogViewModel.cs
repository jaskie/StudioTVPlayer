using System;

namespace StudioTVPlayer.ViewModel
{
    public class HelpDialogViewModel(Action<DialogViewModelBase> closeHandler) : DialogViewModelBase(closeHandler)
    {
    }
}
