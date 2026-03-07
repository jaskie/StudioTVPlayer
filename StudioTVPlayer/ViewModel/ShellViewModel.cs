using MahApps.Metro.Controls.Dialogs;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Providers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace StudioTVPlayer.ViewModel
{
    public class ShellViewModel : ViewModelBase, IDisposable
    {
        private ViewModelBase _currentViewModel;
        private IEnumerable<PlayerControllerViewModel> _playerControllers;

        private ShellViewModel()
        {
            ConfigurationCommand = new UiCommand(Configuration, _ => !(CurrentViewModel is Configuration.ConfigurationViewModel));
            AboutCommand = new UiCommand(About);
            HelpCommand = new UiCommand(Help);
        }

        public static readonly ShellViewModel Instance = new ShellViewModel();

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
            PlayerControllers = GlobalApplicationData.Current.PlayerControllers.Select(playerController => new PlayerControllerViewModel(playerController)).ToArray();
        }

        private void PlayerControllerConnectionStatusChanged(object sender, EventArgs e)
        {
            NotifyPropertyChanged(nameof(AllPlayerControllersConnected));
        }

        public bool IsControllerStatusVisible => CurrentViewModel is Main.PlayoutViewModel && GlobalApplicationData.Current.PlayerControllers.Count > 0;

        public bool AllPlayerControllersConnected => GlobalApplicationData.Current.AllPlayerControllersConnected;

        public IEnumerable<PlayerControllerViewModel> PlayerControllers { get => _playerControllers; private set => Set(ref _playerControllers, value); }

        public async Task ShowPlayoutView()
        {
            if (CurrentViewModel is Main.PlayoutViewModel)
                return;
            if (!await ConfirmCloseAsync())
                return;
            UISBusyState.Set();
            CurrentViewModel = new Main.PlayoutViewModel();
            NotifyPropertyChanged(nameof(IsPlayoutVisible));
            NotifyPropertyChanged(nameof(IsRecordingSchedulerVisible));
            NotifyPropertyChanged(nameof(IsConfigutationViewActive));
        }

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
                UISBusyState.Set();
                CurrentViewModel = new Main.Recording.RecordingSchedulerViewModel();
                NotifyPropertyChanged(nameof(IsRecordingSchedulerVisible));
                NotifyPropertyChanged(nameof(IsPlayoutVisible));
            }
        }

        public UiCommand ConfigurationCommand { get; }

        public UiCommand AboutCommand { get; }

        public UiCommand HelpCommand { get; }

        public void Dispose()
        {
            CurrentViewModel = null;
        }

        public async Task<bool> ConfirmCloseAsync()
        {
            if (CurrentViewModel is ICanClose canClose)
                return await canClose.ConfirmCloseAsync();
            return true;
        }

        private void Configuration(object _)
        {
            if (CurrentViewModel is Configuration.ConfigurationViewModel)
                return;
            CurrentViewModel = new Configuration.ConfigurationViewModel();
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
