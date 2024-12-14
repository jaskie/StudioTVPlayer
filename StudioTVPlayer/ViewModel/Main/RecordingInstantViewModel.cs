using ControlzEx.Standard;
using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main
{
    public sealed class RecordingInstantViewModel : RecordingViewModelBase
    {
        public RecordingInstantViewModel(Model.InputBase input): base(input)
        {
        }

        public RecordingInstantViewModel(Model.RecordingInstant recording) : base(recording)
        {
        }

    }
}
