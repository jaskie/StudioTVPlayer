using System;

namespace StudioTVPlayer.ViewModel
{
    public class HelpDialogViewModel : DialogViewModelBase
    {
        public HelpDialogViewModel(Action<DialogViewModelBase> closeHandler) : base(closeHandler)
        {
        }
    }
}
