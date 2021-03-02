using System;
using System.Collections.Generic;
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

        public IList<MediaPlayer> Players { get; } = new List<MediaPlayer>();

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
            foreach (var player in Players)
                player.Dispose();
            foreach (var channel in Configuration.Channels)
                channel.Dispose();
            foreach (var decklink in DecklinkDevice.EnumerateDevices())
                decklink.Dispose();
        }

        public void UpdateChannels(List<Model.Channel> channels)
        {
            var oldChannels = Configuration.Channels;
            foreach (var channel in channels)
            {
                if (!channel.IsInitialized)
                {
                    channel.Initialize();
                    var player = Players.FirstOrDefault(p => p.Channel == channel);
                    if (player == null)
                        Players.Add(new MediaPlayer(channel));
                    else
                    {
                        var index = Players.IndexOf(player);
                        player.Dispose();
                        Players[index] = new MediaPlayer(channel);
                    }
                }
            }
            foreach (var channel in Configuration.Channels.ToList())
            {
                if (!channels.Contains(channel))
                {
                    Players.FirstOrDefault(p => p.Channel == channel)?.Dispose();
                    channel.Dispose();
                }
            }
            Configuration.Channels = channels;
        }

        public void Initialize()
        {
            foreach(var channel in Configuration.Channels)
            {
                channel.Initialize();
                Players.Add(new MediaPlayer(channel));
            }
        }

    }
}
