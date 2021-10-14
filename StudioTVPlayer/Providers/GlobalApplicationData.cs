using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.Providers
{
    class GlobalApplicationData 
    {
        private const string PathName = "StudioTVPlayer";
        public static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);

        private GlobalApplicationData()
        { }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        
        public IList<MediaPlayer> Players { get; } = new List<MediaPlayer>();


        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            foreach (var player in Players)
                player.Dispose();
            foreach (var channel in Configuration.Current.Channels)
                channel.Dispose();
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
        }

        public void UpdateChannels(List<Channel> channels)
        {
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
            foreach (var channel in Configuration.Current.Channels.ToList())
            {
                if (!channels.Contains(channel))
                {
                    var player = Players.FirstOrDefault(p => p.Channel == channel);
                    if (!(player is null))
                    {
                        player.Dispose();
                        Players.Remove(player);
                    }
                    channel.Dispose();
                }
            }
            Configuration.Current.Channels = channels;
        }

        public void Initialize()
        {
            foreach(var channel in Configuration.Current.Channels)
            {
                channel.Initialize();
                Players.Add(new MediaPlayer(channel));
            }
            foreach (var input in InputList.Current.Inputs)
                input.Initialize();
        }

    }
}
