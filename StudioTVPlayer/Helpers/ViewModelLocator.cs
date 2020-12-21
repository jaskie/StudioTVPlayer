using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Providers;
using StudioTVPlayer.Services;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel.Main.Piloting;
using StudioTVPlayer.ViewModel.Main.Piloting.Browser;
using StudioTVPlayer.ViewModel.Main.Piloting.Player;

namespace StudioTVPlayer.Helpers
{
    public class ViewModelLocator
    {
        static ViewModelLocator()
        {          
            SimpleIoc.Register<IMediaDataProvider, MediaDataProvider>();
            SimpleIoc.Register<INavigationService, NavigationService>();
            SimpleIoc.Register<IPilotingDataProvider, PilotingDataProvider>();
            SimpleIoc.Register<IExchangeService, ExchangeService>();
            SimpleIoc.Register<IUIFocusService, UIFocusService>();


            SimpleIoc.Register<MainViewModel>();
            SimpleIoc.Register<ConfigurationViewModel>();
            SimpleIoc.Register<ConfigurationChannelsViewModel>();
            SimpleIoc.Register<ConfigurationExtensionsViewModel>();
            SimpleIoc.Register<ConfigurationWatchersViewModel>();
            SimpleIoc.Register<BrowserViewModel>();
            SimpleIoc.Register<PilotingViewModel>();
            SimpleIoc.Register<PlayerViewModel>();            
            SimpleIoc.Register<InfoViewModel>();
            SimpleIoc.Register<BrowserTabViewModel>();            ;
            SimpleIoc.Register<BrowserTabItemViewModel>();
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance",
            "CA1822:MarkMembersAsStatic",
            Justification = "This non-static member is needed for data binding purposes.")]

        public MainViewModel Main => SimpleIoc.GetInstance<MainViewModel>();
        
        public BrowserViewModel Browser => SimpleIoc.GetInstance<BrowserViewModel>();
        public PilotingViewModel Piloting => SimpleIoc.GetInstance<PilotingViewModel>();
        public InfoViewModel Info => SimpleIoc.GetInstance<InfoViewModel>();
        public BrowserTabViewModel Ingest => SimpleIoc.GetInstance<BrowserTabViewModel>();      

        public ConfigurationViewModel Configuration => SimpleIoc.GetInstance<ConfigurationViewModel>();
        public ConfigurationWatchersViewModel ConfigurationIngest => SimpleIoc.GetInstance<ConfigurationWatchersViewModel>();
        public ConfigurationChannelsViewModel ConfigurationChannels => SimpleIoc.GetInstance<ConfigurationChannelsViewModel>();
        public ConfigurationExtensionsViewModel ConfigurationMediaExtensions => SimpleIoc.GetInstance<ConfigurationExtensionsViewModel>();
    }
}
