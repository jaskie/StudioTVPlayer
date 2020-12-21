using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ViewModelBase
    {
        private readonly IGlobalApplicationData _globalApplicationData = SimpleIoc.GetInstance<IGlobalApplicationData>();
        private INavigationService _navigationService;
        private IExchangeService _exchangeService;      
       
        private bool _isModified;

        public Model.Configuration Configuration => _globalApplicationData.Configuration;

        public ConfigurationViewModel(IGlobalApplicationData configDataProvider, INavigationService navigationService, IExchangeService vMNotifyService)
        {
            _globalApplicationData = configDataProvider;
            _navigationService = navigationService;
            _exchangeService = vMNotifyService;

            _exchangeService.NotifyOnConfigurationIsModifiedChanged += _exchangeService_NotifyOnConfigurationItemChanged;
            SaveConfigurationCommand = new UiCommand(SaveConfiguration, CanSaveConfiguration);
            CancelCommand = new UiCommand(Cancel);
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }


        private void _exchangeService_NotifyOnConfigurationItemChanged(object sender, ModifiedEventArgs e)
        {
            _isModified = true;
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
                Model.Configuration conf = _globalApplicationData.Configuration;
                conf.Channels = _globalApplicationData.Configuration.Channels;
                conf.Watchers = _globalApplicationData.Configuration.Watchers;
                conf.Extensions = _globalApplicationData.Configuration.Extensions.ToList();
                _exchangeService.RaiseConfigurationChanged(new Model.Args.ConfigurationChangedEventArgs(conf));
                _globalApplicationData.Save();
            }
            _navigationService.SwitchView(Enums.ViewType.Piloting);      
        }

        

    }
}
