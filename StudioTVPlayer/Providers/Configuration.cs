using StudioTVPlayer.Model.Configuration;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class Configuration : Helpers.IPersistable
    {
        private const string ConfigurationFile = "Configuration.xml";

        [XmlArray]
        public List<Model.WatchedFolder> WatchedFolders { get; set; } = [];

        [XmlArray]
        public List<Player> Players { get; set; } = [];

        [XmlArray]
        [XmlArrayItem(typeof(BlackmagicDesignAtemPlayerController))]
        [XmlArrayItem(typeof(ElgatoStreamDeckPlayerController))]
        public List<PlayerControllerBase> PlayerControllers { get; set; } = [];

        public static Configuration Current { get; } = Load();

        private static Configuration Load()
        {
            var configurationFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, ConfigurationFile);
            try
            {
                var configuration = Helpers.DataStore.Load<Configuration>(configurationFile) ?? new();
                foreach (var player in configuration.Players)
                    player.IsModified = false;
                return configuration;
            }
            catch
            {
                return new();
            }
        }

        public void Save()
        {
            var configurationFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, ConfigurationFile);
            Helpers.DataStore.Save(this, configurationFile);
        }
    }
}
