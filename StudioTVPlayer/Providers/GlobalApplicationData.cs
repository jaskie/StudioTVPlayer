using System;
using System.IO;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using TVPlayR;

namespace StudioTVPlayer.Providers
{
    class GlobalApplicationData : IGlobalApplicationData
    {

        private const string PathName = "StudioTVPlayer";
        private const string ConfigurationFile = "configuration.xml";
        private static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);

        public static Configuration LoadConfig()
        {
            var configurationFile = Path.Combine(ApplicationDataDir, ConfigurationFile);
            return DataStore.Load<Configuration>(configurationFile) ?? new Configuration();
        }

        public Configuration Configuration { get; } = LoadConfig();

        public DecklinkDevice[] DecklinkDevices { get; } = DecklinkDevice.EnumerateDevices();

        public VideoFormat[] VideoFormats { get; } = VideoFormat.EnumVideoFormats();

        public PixelFormat[] PixelFormats { get; } = Enum.GetValues(typeof(PixelFormat)).Cast<PixelFormat>().ToArray();

        public void SaveConfiguration()
        {
            var configurationFile = Path.Combine(ApplicationDataDir, ConfigurationFile);
            DataStore.Save(Configuration, configurationFile);
        }
    }
}
