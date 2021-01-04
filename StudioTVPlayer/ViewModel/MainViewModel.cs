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
                ConfigurationCommand = new UiCommand(SwitchToConfiguration);
                PlayoutCommand = new UiCommand(_ => SwitchToPlayout());
                //CurrentViewModel = SimpleIoc.Get<PilotingViewModel>();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Błąd ładowania konfiguracji. " + ex.Message);
            }
            Instance = this;
        }

        public UiCommand PlayoutCommand { get; }
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
            CurrentViewModel = SimpleIoc.Get<ConfigurationViewModel>();
        }
    }
}
