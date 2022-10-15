using StudioTVPlayer.Helpers;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;
using System;

namespace StudioTVPlayer.ViewModel
{
    public class MainViewModel : ViewModelBase, IDisposable
    {
        private ViewModelBase _currentViewModel;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        private MainViewModel()
        {
            ConfigurationCommand = new UiCommand(_ =>
            {
                if (CurrentViewModel is ConfigurationViewModel)
                    return;
                CurrentViewModel = new ConfigurationViewModel();
            });
            AboutCommand = new UiCommand(About);
            HelpCommand = new UiCommand(Help);
        }

        public static readonly MainViewModel Instance = new MainViewModel();

        public ViewModelBase CurrentViewModel
        {
            get => _currentViewModel;
            set
            {
                var oldVm = _currentViewModel;
                if (!Set(ref _currentViewModel, value))
                    return;
                (oldVm as IDisposable)?.Dispose();
            }
        }


        public async void ShowPlayoutView()
        {
            try
            {
                if (CurrentViewModel is PlayoutViewModel)
                    return;
                CurrentViewModel = new PlayoutViewModel();
            }
            catch 
            {
                await _dialogCoordinator.ShowMessageAsync(this, "Configuration error", "Configuration data missing or invalid.\nPlease, configure the application now to use it.");
                CurrentViewModel = new ConfigurationViewModel();
            }
        }

        public UiCommand ConfigurationCommand { get; }

        public UiCommand AboutCommand { get; }
        
        public UiCommand HelpCommand { get; }

        public void Dispose()
        {
            CurrentViewModel = null;
        }

        public bool CanClose()
        {
            return true;
        }

        private async void About(object _)
        {
            var dialog = new MahApps.Metro.Controls.Dialogs.CustomDialog { Title = "About Studio TVPlayer" };
            var dialogVm = new AboutDialogViewModel(instance => _dialogCoordinator.HideMetroDialogAsync(this, dialog));
            dialog.Content = new View.AboutDialog { DataContext = dialogVm };
            await _dialogCoordinator.ShowMetroDialogAsync(this, dialog);
        }

        private async void Help(object _)
        {
            var dialog = new MahApps.Metro.Controls.Dialogs.CustomDialog { Title = "Help" };
            var dialogVm = new HelpDialogViewModel(instance => _dialogCoordinator.HideMetroDialogAsync(this, dialog));
            dialog.Content = new View.HelpDialog { DataContext = dialogVm };
            await _dialogCoordinator.ShowMetroDialogAsync(this, dialog);
        }
    }
}
