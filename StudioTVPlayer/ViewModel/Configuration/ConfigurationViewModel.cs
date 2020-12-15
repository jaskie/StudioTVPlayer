using System.Collections.Generic;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ViewModelBase
    {       
        private IConfigurationDataProvider _configDataProvider;
        private INavigationService _navigationService;
        private IExchangeService _exchangeService;      
       
        public UiCommand SaveConfigurationCommand { get; set; }
        public UiCommand CancelCommand { get; set; }
               
        private bool _isModified;

        public Model.Configuration Configuration { get => Model.Configuration.Instance; }

        public ConfigurationViewModel(IConfigurationDataProvider configDataProvider, INavigationService navigationService, IExchangeService vMNotifyService)
        {
            _configDataProvider = configDataProvider;
            _navigationService = navigationService;
            _exchangeService = vMNotifyService;

            _exchangeService.NotifyOnConfigurationIsModifiedChanged += _exchangeService_NotifyOnConfigurationItemChanged;
            LoadCommands();            
        }

        private void _exchangeService_NotifyOnConfigurationItemChanged(object sender, ModifiedEventArgs e)
        {
            _isModified = true;
        }

        private void LoadCommands()
        {            
            SaveConfigurationCommand = new UiCommand(SaveConfiguration, CanSaveConfiguration);
            CancelCommand = new UiCommand(Cancel);                    
        }

        private bool CanSaveConfiguration(object obj)
        {
            if (_isModified)
                return true;
            return false;
        }
       
        private void Cancel(object obj)
        {
            _navigationService.SwitchView(Enums.ViewType.Piloting);
        }

        private void SaveConfiguration(object obj)
        {          
            if (_isModified)
            {
                Model.Configuration tempConf = new Model.Configuration();
                tempConf.Channels = _configDataProvider.GetChannels();
                tempConf.WatcherMetas = _configDataProvider.GetWatcherMetas();
                tempConf.Extensions = _configDataProvider.GetExtensions().Select(param => (string)param).ToList();
                tempConf.Devices = new List<Device>(Configuration.Devices);
                tempConf.VideoFormats = new List<TVPlayR.VideoFormat>(Configuration.VideoFormats);
                tempConf.PixelFormats = new List<TVPlayR.PixelFormat>(Configuration.PixelFormats);
                                                        
                Model.Configuration.Instance = tempConf;
                _exchangeService.RaiseConfigurationChanged(new Model.Args.ConfigurationChangedEventArgs(tempConf));
                _configDataProvider.Save();
            }
            
            _navigationService.SwitchView(Enums.ViewType.Piloting);      
        }

        

    }
}
