using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.Providers
{
    internal class GlobalApplicationData 
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
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
        }

        public void UpdatePlayers(List<Model.Configuration.Player> players)
        {
            var playersToRemove = RundownPlayers.Where(p => !players.Contains(p.Configuration)).ToList();
            foreach (var runodwnPlayer in playersToRemove)
            {
                runodwnPlayer.Dispose();
                RundownPlayers.Remove(runodwnPlayer);
            }
            foreach (var player in players)
            {
                var rundownPlayer = RundownPlayers.FirstOrDefault(p => p.Configuration == player) ?? new RundownPlayer(player);
                if (!rundownPlayer.IsInitialized)
                {
                    rundownPlayer.Initialize();
                    RundownPlayers.Add(rundownPlayer);
                }
            }
        }

        public void Initialize()
        {
            UpdatePlayers(Configuration.Current.Players);
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
