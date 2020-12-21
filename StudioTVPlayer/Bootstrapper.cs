using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.Providers;

namespace StudioTVPlayer
{
    public class Bootstrapper
    {

        public Bootstrapper()
        {
            SimpleIoc.RegisterSingleton<IGlobalApplicationData>(new GlobalApplicationData());
        }
    }
}
