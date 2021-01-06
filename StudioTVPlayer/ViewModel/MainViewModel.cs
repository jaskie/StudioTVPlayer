using StudioTVPlayer.Helpers;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;

namespace StudioTVPlayer.ViewModel
{
    public class MainViewModel : ViewModelBase
    {
        private ViewModelBase _currentViewModel;
        private readonly MahApps.Metro.Controls.Dialogs.IDialogCoordinator _dialogCoordinator = MahApps.Metro.Controls.Dialogs.DialogCoordinator.Instance;

        public ViewModelBase CurrentViewModel
        {
            get => _currentViewModel;
            set => Set(ref _currentViewModel, value);
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
                CurrentViewModel = new PlayoutViewModel();
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

    }
}
