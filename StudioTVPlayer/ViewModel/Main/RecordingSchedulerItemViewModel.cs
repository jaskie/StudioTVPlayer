using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main
{
    public class RecordingSchedulerItemViewModel : RemovableViewModelBase
    {

        public RecordingSchedulerItemViewModel(RecordingSchedulerItem item)
        {
            UpdateCommand = new UiCommand(Update, _ => IsModified);
        }

        public ICommand UpdateCommand { get; }

        private void Update(object obj)
        {

        }

        public override bool IsValid()
        {
            return true;
        }

    }
}
