using System;
using System.IO;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using TVPlayR;

namespace StudioTVPlayer.Providers
{
    class GlobalApplicationData 
    {

        private const string PathName = "StudioTVPlayer";
        private const string ConfigurationFile = "configuration.xml";
        private static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);

        private GlobalApplicationData()
        { }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        public Configuration Configuration { get; } = LoadConfig();

        public void SaveConfiguration()
        {
            var configurationFile = Path.Combine(ApplicationDataDir, ConfigurationFile);
            DataStore.Save(Configuration, configurationFile);
        }

        private static Configuration LoadConfig()
        {
            var configurationFile = Path.Combine(ApplicationDataDir, ConfigurationFile);
            return DataStore.Load<Configuration>(configurationFile) ?? new Configuration();
        }

        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            foreach (var decklink in DecklinkDevice.EnumerateDevices())
                decklink.Dispose();
        }

    }
}
