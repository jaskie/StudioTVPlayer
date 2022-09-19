﻿using System;
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
        {
            EncoderPresets = LoadEncoderPresets();
        }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        
        public IList<RundownPlayer> RundownPlayers { get; } = new List<RundownPlayer>();
        
        public IEnumerable<EncoderPreset> EncoderPresets { get; private set; }

        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            foreach (var player in RundownPlayers)
                player.Dispose();
            foreach (var channel in Configuration.Current.Players)
                channel.Dispose();
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
        }

        public void UpdatePlayers(List<RundownPlayer> players)
        {
            foreach (var player in players)
            {
                if (!player.IsInitialized)
                {
                    player.Initialize();
                    var rundownPlayer = RundownPlayers.FirstOrDefault(p => p == player);
                    if (rundownPlayer == null)
                        RundownPlayers.Add(player);
                    else
                    {
                        var index = RundownPlayers.IndexOf(rundownPlayer);
                        rundownPlayer.Dispose();
                        RundownPlayers[index] = player;
                    }
                }
            }
            foreach (var player in Configuration.Current.Players.ToList())
            {
                if (!players.Contains(player))
                {
                    var mediaPlayer = RundownPlayers.FirstOrDefault(p => p == player);
                    if (!(mediaPlayer is null))
                    {
                        mediaPlayer.Dispose();
                        RundownPlayers.Remove(mediaPlayer);
                    }
                    player.Dispose();
                }
            }
            Configuration.Current.Players = players;
        }

        public void Initialize()
        {
            foreach(var player in Configuration.Current.Players)
            {
                player.Initialize();
                RundownPlayers.Add(player);
            }
            foreach (var input in InputList.Current.Inputs)
                input.Initialize();
        }


        public IEnumerable<EncoderPreset> LoadEncoderPresets()
        {
            var assembly = System.Reflection.Assembly.GetExecutingAssembly();
            var resourceStream = assembly.GetManifestResourceStream($"{assembly.GetName().Name}.Resources.EmbeddedPresets.xml");
            var serializer = new System.Xml.Serialization.XmlSerializer(typeof(List<EncoderPreset>), new System.Xml.Serialization.XmlRootAttribute("PresetList"));
            var presets = (List<EncoderPreset>)serializer.Deserialize(resourceStream);
            presets.ForEach(p => p.IsEmbedded = true);
            return presets;
        }

    }
}
