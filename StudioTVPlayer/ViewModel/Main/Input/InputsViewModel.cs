using MahApps.Metro.Controls.Dialogs;
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
                InputViewModelBase inputViewModel = input switch
                {
                    Model.DecklinkInput decklink => new DecklinkInputViewModel(decklink),
                    _ => throw new ApplicationException("Invalid model type provided"),
                };
                inputViewModel.RemoveRequested += Input_RemoveRequested;
               return inputViewModel;
            }));
        }

        public ICommand AddDecklinkInputCommand { get; }

        public IList<InputViewModelBase> Inputs { get; }

        public bool CanAddInput => InputList.Current.CanAddDecklinkInput;

        private void AddDecklinkInput(object _)
        {
            var input = InputList.Current.AddDecklinkInput();
            var vm = new DecklinkInputViewModel(input);
            vm.RemoveRequested += Input_RemoveRequested;
            Inputs.Add(vm);
        }

        private async void Input_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as InputViewModelBase ?? throw new ArgumentException($"{nameof(InputViewModelBase)} expected, {sender?.GetType()} got.");
            if (RecordingStore.Current.RunningRecordings.Any(r => r.Input == vm.Input))
            {
                await ShellViewModel.Instance.ShowMessageAsync("Remove input", "You cannot remove input with ongoing recording(s).", MessageDialogStyle.Affirmative);
                return;
            }
            if ((Model.RecordingScheduler.Current.Recordings.Any(r => r.InputId == vm.Input.Id) &&
                await ShellViewModel.Instance.ShowMessageAsync("Remove input", "There are scheduled recordings for this input.\n\nDo you really want to remove it?", MessageDialogStyle.AffirmativeAndNegative) == MessageDialogResult.Affirmative)
                ||
                await ShellViewModel.Instance.ShowMessageAsync("Remove input", "Do you really want to remove this input?", MessageDialogStyle.AffirmativeAndNegative) == MessageDialogResult.Affirmative)
            {
                if (InputList.Current.RemoveInput(vm.Input))
                {
                    Inputs.Remove(vm);
                    vm.Dispose();
                    vm.RemoveRequested -= Input_RemoveRequested;
                }
            }
        }

        public void Dispose()
        {
            foreach (var input in Inputs)
                input.Dispose();
        }
    }
}
