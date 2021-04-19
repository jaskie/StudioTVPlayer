using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel
{
    public class AboutDialogViewModel: ViewModelBase
    {
        public AboutDialogViewModel(Action<AboutDialogViewModel> closeHandler)
        {
            CloseCommand = new UiCommand(_ => closeHandler(this));
        }

        public UiCommand CloseCommand { get; }
    }
}
