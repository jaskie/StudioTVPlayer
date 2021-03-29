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

        public static readonly MainViewModel Instance = new MainViewModel();

        private MainViewModel() 
        {
            ConfigurationCommand = new UiCommand(SwitchToConfiguration);
        }

        public async void LoadConfiguraion()
        {
            try
            {
                SwitchToPlayout();
            }
            catch 
            {
                await _dialogCoordinator.ShowMessageAsync(this, "Configuration error", "Configuration data missing or invalid.\nPlease, configure the application now to use it.");
                CurrentViewModel = new ConfigurationViewModel();
            }
        }

        public UiCommand ConfigurationCommand { get; }

        public void SwitchToPlayout()
        {
            if (CurrentViewModel is PlayoutViewModel)
                return;
            CurrentViewModel = new PlayoutViewModel();
        }

        private void SwitchToConfiguration(object _)
        {
            if (CurrentViewModel is ConfigurationViewModel)
                return;
            CurrentViewModel = new ConfigurationViewModel();
        }

        public void Dispose()
        {
            CurrentViewModel = null;
        }
    }
}
