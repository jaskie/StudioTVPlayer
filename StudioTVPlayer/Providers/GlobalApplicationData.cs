using System.Collections.Generic;
using System.Linq;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using StudioTVPlayer.Model.Interfaces;
using StudioTVPlayer.ViewModel.Configuration;

namespace StudioTVPlayer.Providers
{
    class GlobalApplicationData : IGlobalApplicationData
    {
        private const string _configurationFile = "configuration";

        public static Configuration LoadConfig()
        {
            return DataStore.Load<Configuration>(_configurationFile) ?? new Configuration();
        }

        public Configuration Configuration { get; } = LoadConfig();

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
            DataStore.Save(Configuration, _configurationFile);
        }
    }
}
