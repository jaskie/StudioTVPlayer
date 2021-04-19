using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
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

        public VersionInfo VersionInfo => VersionInfo.Current;

        public UiCommand CloseCommand { get; }
    }
}
