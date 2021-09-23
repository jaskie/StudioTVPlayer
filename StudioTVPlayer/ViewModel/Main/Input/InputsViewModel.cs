using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
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
            Inputs = new ObservableCollection<DecklinkInputViewModel>(InputList.Current.Inputs.Select( input =>
            {
                switch (input)
                {
                    case Model.DecklinkInput decklink:
                        return new DecklinkInputViewModel(decklink);
                    default:
                        throw new ApplicationException("Invalid model type provided");
                }
            }));
        }

        public ICommand AddDecklinkInputCommand { get; }

        private void AddDecklinkInput(object _)
        {
            var input = new Model.DecklinkInput();
            InputList.Current.Inputs.Add(input);
            var vm = new DecklinkInputViewModel(input);
            vm.RemoveRequested += Input_RemoveRequested;
            Inputs.Add(vm);
        }

        public IList<DecklinkInputViewModel> Inputs { get; }
        
        private void Input_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as DecklinkInputViewModel ?? throw new ArgumentException(nameof(sender));
            if (InputList.Current.Inputs.Remove(vm.Input))
                Inputs.Remove(vm);
        }
    }
}
