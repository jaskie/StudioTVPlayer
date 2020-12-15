using System;
using System.Windows;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main.Piloting;

namespace StudioTVPlayer.ViewModel.Main
{
    public class MainViewModel : ViewModelBase
    {
        private ViewModelBase _currentViewModel;
        public ViewModelBase CurrentViewModel
        {
            get { return _currentViewModel; }
            
            set
            { 
                Set(ref _currentViewModel, value);
            }
        }

        public UiCommand SwitchToPilotingCommand { get; set; }
        public UiCommand SwitchToConfigurationCommand { get; set; }

        private IConfigurationDataProvider _configurationDataProvider { get; }
        private INavigationService _navigationService { get; }

        public MainViewModel(IConfigurationDataProvider configurationDataProvider, INavigationService navigationService)
        {
            _configurationDataProvider = configurationDataProvider;
            _navigationService = navigationService;
            try
            {
                Model.Configuration.Instance = _configurationDataProvider.LoadConfig();
                SwitchToConfigurationCommand = new UiCommand(SwitchToConfiguration);
                SwitchToPilotingCommand = new UiCommand(SwitchToPiloting);
                CurrentViewModel = SimpleIoc.GetInstance<PilotingViewModel>();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Błąd ładowania konfiguracji. " + ex.Message);
            }                    
        }

        private void SwitchToPiloting(object obj)
        {
            _navigationService.SwitchView(Enums.ViewType.Piloting);            
        }

        private void SwitchToConfiguration(object obj)
        {
            _navigationService.SwitchView(Enums.ViewType.Configuration);
        }
    }
}
