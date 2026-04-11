using MahApps.Metro.Controls.Dialogs;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel
{
    public sealed class ShellViewModel : ViewModelBase, IDisposable
    {
        private ViewModelBase _currentViewModel;
        private IEnumerable<PlayerControllerViewModel> _playerControllers;

        private ShellViewModel()
        {
            ConfigurationCommand = new UiCommand(_ => Task.Run(async () => await ShowView<Configuration.ConfigurationViewModel>()), _ => CurrentViewModel is not Configuration.ConfigurationViewModel);
            PlayoutCommand = new UiCommand(_ => Task.Run(async () => await ShowPlayoutView()), _ => CurrentViewModel is not Main.PlayoutViewModel);
            RecordingSchedulerCommand = new UiCommand(_ => Task.Run(async () => await ShowView<Main.Recording.RecordingSchedulerViewModel>()), _ => CurrentViewModel is not Main.Recording.RecordingSchedulerViewModel);
            AboutCommand = new UiCommand(About);
            HelpCommand = new UiCommand(Help);
        }

        public static readonly ShellViewModel Instance = new();

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
                await ShowPlayoutView();
            }
            catch (Exception e)
            {
                await ShowMessageAsync("Initialization failed", 
                    $"Application failed to initialize. It may be a configuration problem.\n\nPlease, review the configuration considering following error:\n{(e.InnerException ?? e).Message}");
                CurrentViewModel = new Configuration.ConfigurationViewModel();
            }
        }

        private void PlayerControllersModified(object sender, EventArgs e)
        {
            if (PlayerControllers != null)
                foreach (var playerControler in PlayerControllers)
                    playerControler.Dispose();
            PlayerControllers = [.. GlobalApplicationData.Current.PlayerControllers.Select(playerController => new PlayerControllerViewModel(playerController))];
        }

        private void PlayerControllerConnectionStatusChanged(object sender, EventArgs e)
        {
            NotifyPropertyChanged(nameof(AllPlayerControllersConnected));
        }

        public bool IsControllerStatusVisible => CurrentViewModel is Main.PlayoutViewModel && GlobalApplicationData.Current.PlayerControllers.Count > 0;

        public bool AllPlayerControllersConnected => GlobalApplicationData.Current.AllPlayerControllersConnected;

        public IEnumerable<PlayerControllerViewModel> PlayerControllers { get => _playerControllers; private set => Set(ref _playerControllers, value); }

        public bool IsConfigutationViewActive => CurrentViewModel is Configuration.ConfigurationViewModel;

        public bool IsPlayoutVisible
        {
            get => CurrentViewModel is Main.PlayoutViewModel;
            set
            {
                if (!value || CurrentViewModel is Main.PlayoutViewModel)
                    return;
                _ = ShowPlayoutView();
            }
        }

        public bool IsRecordingSchedulerVisible
        {
            get => CurrentViewModel is Main.Recording.RecordingSchedulerViewModel;
            set
            {
                if (!value || CurrentViewModel is Main.Recording.RecordingSchedulerViewModel)
                    return;
                _ = ShowView<Main.Recording.RecordingSchedulerViewModel>();
            }
        }

        public ICommand PlayoutCommand { get; }
        public ICommand RecordingSchedulerCommand { get; }
        public ICommand ConfigurationCommand { get; }
        public ICommand AboutCommand { get; }
        public ICommand HelpCommand { get; }

        public void Dispose()
        {
        CurrentViewModel = null;
        }

        public async Task<bool> ConfirmCloseAsync()
        {
            if (CurrentViewModel is IConfirmClose canClose)
                return await canClose.ConfirmCloseAsync();
            return true;
        }

        public async Task ShowPlayoutView()
        {
            await ShowView<Main.PlayoutViewModel>();
        }

        private async Task ShowView<T>() where T: ViewModelBase, new()
        {
            if (CurrentViewModel is T)
                return;
            if (!await ConfirmCloseAsync())
                return;
            UIBusyState.Set();
            CurrentViewModel = new T();
            NotifyPropertyChanged(nameof(IsPlayoutVisible));
            NotifyPropertyChanged(nameof(IsRecordingSchedulerVisible));
            NotifyPropertyChanged(nameof(IsConfigutationViewActive));
        }

        private async void About(object _)
        {
            var dialog = new CustomDialog { Title = "About Studio TVPlayer" };
            var dialogVm = new AboutDialogViewModel(instance => DialogCoordinator.Instance.HideMetroDialogAsync(this, dialog));
            dialog.Content = new View.AboutDialog { DataContext = dialogVm };
            await DialogCoordinator.Instance.ShowMetroDialogAsync(this, dialog);
        }

        private async void Help(object _)
        {
            var dialog = new MahApps.Metro.Controls.Dialogs.CustomDialog { Title = "Help" };
            var dialogVm = new HelpDialogViewModel(instance => DialogCoordinator.Instance.HideMetroDialogAsync(this, dialog));
            dialog.Content = new View.HelpDialog { DataContext = dialogVm };
            await DialogCoordinator.Instance.ShowMetroDialogAsync(this, dialog);
        }

        public async Task<MessageDialogResult> ShowMessageAsync(string title, string message, MessageDialogStyle messageDialogStyle = MessageDialogStyle.Affirmative, MetroDialogSettings dialogSettings = null)
        {
            return await DialogCoordinator.Instance.ShowMessageAsync(this, title, message, messageDialogStyle, dialogSettings);
        }
    }
}
