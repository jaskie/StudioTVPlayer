using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel
{
    public class HelpDialogViewModel : DialogViewModelBase
    {
        public HelpDialogViewModel(Action<DialogViewModelBase> closeHandler) : base(closeHandler)
        {
        }
    }
}
