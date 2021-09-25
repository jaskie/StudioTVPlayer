using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Main.Input
{
    public class InputsViewModel: ViewModelBase, IDisposable
    {
        public InputsViewModel()
        {
            AddDecklinkInputCommand = new UiCommand(AddDecklinkInput);
            Inputs = new ObservableCollection<InputViewModelBase>(InputList.Current.Inputs.Select(input =>
           {
               InputViewModelBase inputViewModel;
               switch (input)
               {
                   case Model.DecklinkInput decklink:
                       inputViewModel = new DecklinkInputViewModel(decklink);
                       break;
                   default:
                       throw new ApplicationException("Invalid model type provided");
               }
               inputViewModel.RemoveRequested += Input_RemoveRequested;
               return inputViewModel;
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

        public IList<InputViewModelBase> Inputs { get; }
        
        private void Input_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as DecklinkInputViewModel ?? throw new ArgumentException(nameof(sender));
            if (InputList.Current.RemoveInput(vm.Input))
            {
                Inputs.Remove(vm);
                vm.Dispose();
                vm.RemoveRequested -= Input_RemoveRequested;
            }
        }

        public void Dispose()
        {
            foreach (var input in Inputs)
                input.Dispose();
        }
    }
}
