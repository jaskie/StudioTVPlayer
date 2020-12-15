using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Configuration;
using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel.Main.Piloting;
using static StudioTVPlayer.Model.Enums;

namespace StudioTVPlayer.Services
{
    public class NavigationService : INavigationService
    {     
        public void SwitchView(ViewType view)
        {
            MainViewModel Main = SimpleIoc.GetInstance<MainViewModel>();

            switch(view)
            {
                case ViewType.Configuration:
                    {
                        Main.CurrentViewModel = SimpleIoc.GetInstance<ConfigurationViewModel>();
                        break;
                    }

                case ViewType.Piloting:
                    {
                        Main.CurrentViewModel = SimpleIoc.GetInstance<PilotingViewModel>();
                        SimpleIoc.Destroy<ConfigurationViewModel>();
                        SimpleIoc.Destroy<ConfigurationWatchersViewModel>();
                        SimpleIoc.Destroy<ConfigurationExtensionsViewModel>();
                        SimpleIoc.Destroy<ConfigurationChannelsViewModel>();
                        break;
                    }
            }
        }
    }
}
