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
            var input = new Model.DecklinkInput();
            var vm = new DecklinkInputViewModel(input);
            vm.RemoveRequested += Input_RemoveRequested;
            Inputs.Add(vm);
        }

        public IList<DecklinkInputViewModel> Inputs { get; }
        
        private void Input_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as DecklinkInputViewModel ?? throw new ArgumentException(nameof(sender));
            Inputs.Remove(vm);
        }
    }
}
