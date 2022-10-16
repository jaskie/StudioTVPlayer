using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model.Configuration;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class Configuration
    {
        private const string ConfigurationFile = "configuration.xml";

        [XmlArray]
        public List<Model.WatchedFolder> WatchedFolders { get; set; } = new List<Model.WatchedFolder>();

        [XmlArray]
        public List<Player> Players { get; set; } = new List<Player>();

        public static Configuration Current { get; } = Load();

        private static Configuration Load()
        {
            var configurationFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, ConfigurationFile);
            try
            {
                var configuration = DataStore.Load<Configuration>(configurationFile) ?? new Configuration();
                foreach (var player in configuration.Players)
                    player.IsModified = false;
                return configuration;
            }
            catch
            {
                return new Configuration();
            }
        }

        public void Save()
        {
            var configurationFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, ConfigurationFile);
            DataStore.Save(this, configurationFile);
        }

    }
}
