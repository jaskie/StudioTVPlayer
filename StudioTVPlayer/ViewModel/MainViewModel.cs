using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;
using System;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel
{
    public class MainViewModel : ViewModelBase, IDisposable
    {
        private ViewModelBase _currentViewModel;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        private MainViewModel()
        {
            ConfigurationCommand = new UiCommand(Configure);
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

        public async void InitializeAndShowPlayoutView()
        {
            try
            {
                GlobalApplicationData.Current.Initialize();
                CurrentViewModel = new PlayoutViewModel();
            }
            catch (Exception e)
            {
                await ShowMessageAsync("Configuration error", $"Configuration data is missing or invalid.\nPlease configure the application to use it.\n\nFollowing error occured:\n{(e.InnerException ?? e).Message}");
                CurrentViewModel = new ConfigurationViewModel();
            }
        }

        public void ShowPlayoutView()
        {
            if (CurrentViewModel is PlayoutViewModel)
                return;
            CurrentViewModel = new PlayoutViewModel();
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

        private void Configure(object _)
        {
            if (CurrentViewModel is ConfigurationViewModel)
                return;
            CurrentViewModel = new ConfigurationViewModel();
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

        public async Task ShowMessageAsync(string title, string message)
        {
            await _dialogCoordinator.ShowMessageAsync(this, title, message);
        }
    }
}
