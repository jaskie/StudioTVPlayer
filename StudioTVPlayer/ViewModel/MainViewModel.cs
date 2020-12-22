using System;
using System.Windows;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;

namespace StudioTVPlayer.ViewModel
{
    public class MainViewModel : ViewModelBase
    {
        private ViewModelBase _currentViewModel;

        public ViewModelBase CurrentViewModel
        {
            get => _currentViewModel;
            set => Set(ref _currentViewModel, value);
        }

        public static MainViewModel Instance { get; private set; }

        public MainViewModel()
        {
            try
            {
                SwitchToConfigurationCommand = new UiCommand(SwitchToConfiguration);
                SwitchToPilotingCommand = new UiCommand(SwitchToPiloting);
                CurrentViewModel = SimpleIoc.Get<PilotingViewModel>();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Błąd ładowania konfiguracji. " + ex.Message);
            }
            Instance = this;
        }

        public UiCommand SwitchToPilotingCommand { get; }
        public UiCommand SwitchToConfigurationCommand { get; }

        private void SwitchToPiloting(object _)
        {
            if (CurrentViewModel is PilotingViewModel)
                return;
            CurrentViewModel = SimpleIoc.Get<PilotingViewModel>();
        }

        private void SwitchToConfiguration(object _)
        {
            if (CurrentViewModel is ConfigurationViewModel)
                return;
            CurrentViewModel = SimpleIoc.Get<ConfigurationViewModel>();
        }
    }
}
