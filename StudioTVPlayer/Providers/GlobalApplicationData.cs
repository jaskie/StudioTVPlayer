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
        private readonly List<RundownPlayer> _rundownPlayers = new List<RundownPlayer>();

        private GlobalApplicationData()
        {
            EncoderPresets = LoadEncoderPresets();
        }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        public IReadOnlyList<RundownPlayer> RundownPlayers => _rundownPlayers;
        
        public IEnumerable<EncoderPreset> EncoderPresets { get; private set; }

        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            foreach (var player in RundownPlayers)
                player.Dispose();
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
        }

        public void UpdatePlayers(List<Model.Configuration.Player> playersConfiguration)
        {
            var rundownPlayers = RundownPlayers.Where(p => !playersConfiguration.Contains(p.Configuration)).ToList();
            foreach (var rundownPlayer in rundownPlayers)
            {
                rundownPlayer.Dispose();
                _rundownPlayers.Remove(rundownPlayer);
            }
            rundownPlayers.Clear();
            foreach (var player in playersConfiguration)
            {
                var rundownPlayer = RundownPlayers.FirstOrDefault(p => p.Configuration == player) ?? new RundownPlayer(player);
                if (player.IsModified)
                    rundownPlayer.Uninitialize();
                if (!rundownPlayer.IsInitialized)
                    rundownPlayer.Initialize();
                rundownPlayers.Add(rundownPlayer);
            }
            _rundownPlayers.Clear();
            _rundownPlayers.AddRange(rundownPlayers);
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
