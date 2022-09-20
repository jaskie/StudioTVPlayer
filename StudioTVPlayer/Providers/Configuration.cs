using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class Configuration
    {
        private const string ConfigurationFile = "configuration.xml";
        private List<Model.Configuration.Player> _players = new List<Model.Configuration.Player>();

        [XmlArray]
        [XmlArrayItem("WatchedFolder")]
        public List<WatchedFolder> WatchedFolders { get; set; } = new List<WatchedFolder>();

        [XmlArray]
        public List<Model.Configuration.Player> Players
        {
            get => _players; 
            set
            {
                foreach (var player in value)
                    player.IsModified = false;
                _players = value;
            }
        }
        public static Configuration Current { get; } = Load();

        private static Configuration Load()
        {
            var configurationFile = Path.Combine(GlobalApplicationData.ApplicationDataDir, ConfigurationFile);
            try
            {
                return DataStore.Load<Configuration>(configurationFile) ?? new Configuration();
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
