using MahApps.Metro.Controls.Dialogs;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel
{
    public class MainViewModel : ViewModelBase, IDisposable
    {
        private ViewModelBase _currentViewModel;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        private MainViewModel()
        {
            ConfigurationCommand = new UiCommand(Configuration, _ => !(CurrentViewModel is ConfigurationViewModel));
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
                NotifyPropertyChanged(nameof(IsControllerStatusVisible));
            }
        }

        public async void InitializeAndShowPlayoutView()
        {
            GlobalApplicationData.Current.PlayerControllerConnectionStatusChanged += PlayerControllerConnectionStatusChanged;
            GlobalApplicationData.Current.PlayerControllersModified += PlayerControllersModified;
            try
            {
                GlobalApplicationData.Current.Initialize();
                CurrentViewModel = new PlayoutViewModel();
            }
            catch (Exception e)
            {
                await ShowMessageAsync("Initialization failed", 
                    $"Application failed to initialize. It may be a configuration problem.\n\nPlease, review the configuration considering following error:\n{(e.InnerException ?? e).Message}");
                CurrentViewModel = new ConfigurationViewModel();
            }
        }

        private void PlayerControllersModified(object sender, EventArgs e)
        {
            foreach (var playerControler in PlayerControllers)
                playerControler.Dispose();
            PlayerControllers = GlobalApplicationData.Current.PlayerControllers.Select(playerController => new PlayerControllerViewModel(playerController)).ToArray();
            NotifyPropertyChanged(nameof(PlayerControllers));
        }

        private void PlayerControllerConnectionStatusChanged(object sender, EventArgs e)
        {
            NotifyPropertyChanged(nameof(AllPlayerControllersConnected));
        }

        public bool IsControllerStatusVisible => CurrentViewModel is PlayoutViewModel && GlobalApplicationData.Current.PlayerControllers.Count > 0;

        public bool AllPlayerControllersConnected => GlobalApplicationData.Current.AllPlayerControllersConnected;

        public IEnumerable<PlayerControllerViewModel> PlayerControllers { get; private set; } = Array.Empty<PlayerControllerViewModel>();

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

        private void Configuration(object _)
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

        public async Task<MessageDialogResult> ShowMessageAsync(string title, string message, MessageDialogStyle messageDialogStyle = MessageDialogStyle.Affirmative, MetroDialogSettings dialogSettings = null)
        {
            return await _dialogCoordinator.ShowMessageAsync(this, title, message, messageDialogStyle, dialogSettings);
        }
    }
}
