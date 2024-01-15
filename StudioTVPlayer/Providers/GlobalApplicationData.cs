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
        private List<PlayerControllerBase> _playerControllers = new List<PlayerControllerBase>();
        private List<Recording> _recordings = new List<Recording>();

        private GlobalApplicationData()
        {
            EncoderPresets = LoadEncoderPresets();
        }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        public IReadOnlyList<RundownPlayer> RundownPlayers => _rundownPlayers;

        public IReadOnlyList<PlayerControllerBase> PlayerControllers => _playerControllers;

        public IReadOnlyList<Recording> Recordings { get => _recordings; }

        public IReadOnlyList<EncoderPreset> EncoderPresets { get; private set; }

        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            foreach (var recording in _recordings.ToList())
                recording.Dispose(); //will also call Recording_Finished below
            foreach (var player in RundownPlayers)
                player.Dispose();
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
            foreach (var playerController in PlayerControllers)
                playerController.Dispose();
        }

        public void UpdatePlayers(List<PlayerUpdateItem> newConfiguration)
        {
            var rundownPlayers = RundownPlayers.Where(p => !newConfiguration.Select(x => x.Player).Contains(p.Configuration)).ToList();
            foreach (var rundownPlayer in rundownPlayers)
            {
                rundownPlayer.Dispose();
                _rundownPlayers.Remove(rundownPlayer);
            }
            rundownPlayers.Clear();
            foreach (var player in newConfiguration)
            {
                var rundownPlayer = RundownPlayers.FirstOrDefault(p => p.Configuration == player.Player) ?? new RundownPlayer(player.Player);
                if (player.NeedsReinitialization)
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
            UpdatePlayers(Configuration.Current.Players.Select(p => new PlayerUpdateItem { Player = p, NeedsReinitialization = true }).ToList());
            foreach (var input in InputList.Current.Inputs)
                input.Initialize();
            UpdatePlayerControllers();
        }

        public void UpdatePlayerControllers()
        {
            foreach (var playerController in _playerControllers)
                playerController.Dispose();
            _playerControllers = Configuration.Current.PlayerControllers.Select(CreatePlayerController).ToList();
        }

        private PlayerControllerBase CreatePlayerController(Model.Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            switch (playerControllerConfiguration)
            {
                case Model.Configuration.BlackmagicDesignAtemPlayerController bmdPlayerControllerConfiguration:
                    return new BlackmagicDesignAtemPlayerController(bmdPlayerControllerConfiguration);
                default: throw new ApplicationException($"Unknown player controller ({playerControllerConfiguration})");
            }
        }

        public IReadOnlyList<EncoderPreset> LoadEncoderPresets()
        {
            var assembly = System.Reflection.Assembly.GetExecutingAssembly();
            var resourceStream = assembly.GetManifestResourceStream($"{assembly.GetName().Name}.Resources.EmbeddedPresets.xml");
            var serializer = new System.Xml.Serialization.XmlSerializer(typeof(List<EncoderPreset>), new System.Xml.Serialization.XmlRootAttribute("PresetList"));
            var presets = (List<EncoderPreset>)serializer.Deserialize(resourceStream);
            return presets;
        }

        public void AddRecording(Recording recording)
        {
            _recordings.Add(recording);
            recording.Finished += Recording_Finished;
        }

        private void Recording_Finished(object sender, EventArgs e)
        {
            var recording = sender as Recording ?? throw new ArgumentException(nameof(sender));
            recording.Finished -= Recording_Finished;
            _recordings.Remove(recording);
        }
    }

    internal class PlayerUpdateItem
    {
        public Model.Configuration.Player Player;
        public bool NeedsReinitialization;
    }
}
