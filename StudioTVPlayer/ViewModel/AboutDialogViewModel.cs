﻿using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
