using StudioTVPlayer.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class InputsViewModel: ViewModelBase
    {
        public InputsViewModel()
        {
            AddDecklinkInputCommand = new UiCommand(AddDecklinkInput);
            Inputs = new ObservableCollection<DecklinkInputViewModel>();
        }

        public ICommand AddDecklinkInputCommand { get; }

        private void AddDecklinkInput(object _)
        {

        }

        public IList<DecklinkInputViewModel> Inputs { get; }

    }
}
