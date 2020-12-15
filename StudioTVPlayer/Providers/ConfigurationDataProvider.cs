using System.Collections.Generic;
using System.IO;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Configuration;

namespace StudioTVPlayer.Providers
{
    class ConfigurationDataProvider : IConfigurationDataProvider
    {
        private string _configurationFile = "configuration";
        private static string _defaultWatchersPath = @"c:\StudioTVPlayer\";
        private static string _defaultFilteredWatcherPath = Path.Combine(_defaultWatchersPath, "Ingest");
        private static string _defaultWatcherPath = Path.Combine(_defaultWatchersPath, "Oprawa");

        public Configuration LoadConfig()
        {
            Configuration temp = DataStore.Load<Configuration>(_configurationFile);
            if (temp != null)
            {
                temp.Devices = LoadDevices();

                foreach (Channel channel in temp.Channels)
                {
                    channel.VideoFormat = temp.VideoFormats.FirstOrDefault((f) => f.Id == channel.VideoFormatId);
                    channel.Device = temp.Devices.FirstOrDefault(p => p.ID == channel.Device.ID);

                    channel.Init(channel.VideoFormatId, channel.PixelFormat, channel.AudioChannelsCount);
                    channel.AddOutput(TVPlayR.DecklinkDevice.EnumerateDevices().FirstOrDefault(f => f.Index == channel.Device.ID));
                }

                return temp;
            }

            //default

            if (!Directory.Exists(_defaultWatchersPath))
                Directory.CreateDirectory(_defaultWatchersPath);

            if (!Directory.Exists(_defaultFilteredWatcherPath))
                    Directory.CreateDirectory(_defaultFilteredWatcherPath);

            if (!Directory.Exists(_defaultWatcherPath))
                Directory.CreateDirectory(_defaultWatcherPath);

            Configuration.Instance.Devices = LoadDevices();
            Configuration.Instance.WatcherMetas.Add(new WatcherMeta {Name = "Główne", Path = _defaultFilteredWatcherPath, IsFiltered = true });
            Configuration.Instance.WatcherMetas.Add(new WatcherMeta {Name = "Oprawa", Path = _defaultWatcherPath, IsFiltered = false });

            Configuration.Instance.Extensions.Add(".mxf");
            Configuration.Instance.Extensions.Add(".mov");
            Configuration.Instance.Extensions.Add(".mp4");
            Configuration.Instance.Save(_configurationFile);


            return Configuration.Instance;
        }

        private List<Device> LoadDevices()
        {          
            var Devices = TVPlayR.DecklinkDevice.EnumerateDevices();

            if (!(Devices.Length > 0))
                return null;

            List<Device> LoadedDevices = new List<Device>();

            foreach (var device in Devices)
            {
                LoadedDevices.Add(new Device { ID = device.Index, Name = device.DisplayName });
            }

            return LoadedDevices;          
        }

        public void Save()
        {
            Configuration.Instance.Save(_configurationFile);
        }

        public List<WatcherMeta> GetWatcherMetas()
        {
            return SimpleIoc.GetInstance<ConfigurationWatchersViewModel>().WatcherMetas.ToList();
        }

        public List<Channel> GetChannels()
        {
            return SimpleIoc.GetInstance<ConfigurationChannelsViewModel>().Channels.ToList();
        }

        public List<StringWrapper> GetExtensions()
        {
            return SimpleIoc.GetInstance<ConfigurationExtensionsViewModel>().Extensions.ToList();
        }
    }
}
