using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Main;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ConfigurationViewModel : ViewModelBase
    {
        private readonly IGlobalApplicationData _globalApplicationData;
        private readonly IExchangeService _exchangeService;      
       
        private bool _isModified;

        public ConfigurationViewModel(IGlobalApplicationData globalApplicationData, IExchangeService vMNotifyService)
        {
            _globalApplicationData = globalApplicationData;
            _exchangeService = vMNotifyService;
            _exchangeService.NotifyOnConfigurationIsModifiedChanged += _exchangeService_NotifyOnConfigurationItemChanged;
            SaveConfigurationCommand = new UiCommand(SaveConfiguration, CanSaveConfiguration);
            CancelCommand = new UiCommand(Cancel);
        }

        public UiCommand SaveConfigurationCommand { get; }

        public UiCommand CancelCommand { get; }

        public ChannelsViewModel ChannelsViewModel { get; } = SimpleIoc.Get<ChannelsViewModel>();

        public ExtensionsViewModel ExtensionsViewModel { get; }


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
            MainViewModel.Instance.CurrentViewModel = SimpleIoc.Get<PilotingViewModel>();
        }

        private void SaveConfiguration(object obj)
        {
            if (_isModified)
            {
                Model.Configuration conf = _globalApplicationData.Configuration;
                conf.Channels = _globalApplicationData.Configuration.Channels;
                conf.WatchedFolders = _globalApplicationData.Configuration.WatchedFolders;
                conf.Extensions = _globalApplicationData.Configuration.Extensions.ToList();
                _exchangeService.RaiseConfigurationChanged(new Model.Args.ConfigurationChangedEventArgs(conf));
                _globalApplicationData.SaveConfiguration();
            }
            MainViewModel.Instance.CurrentViewModel = SimpleIoc.Get<PilotingViewModel>();
        }
    }
}
