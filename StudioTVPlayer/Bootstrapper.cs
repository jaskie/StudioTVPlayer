using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Providers;
using StudioTVPlayer.Services;

namespace StudioTVPlayer
{
    public class Bootstrapper
    {
        public Bootstrapper()
        {
            SimpleIoc.RegisterSingleton<IGlobalApplicationData>(new GlobalApplicationData());

            // services
            SimpleIoc.Register<IMediaDataProvider, MediaDataProvider>();
            SimpleIoc.Register<IPilotingDataProvider, PilotingDataProvider>();
            SimpleIoc.Register<IExchangeService, ExchangeService>();
            SimpleIoc.Register<IUIFocusService, UIFocusService>();
        }

    }
}
