using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using StudioTVPlayer.Model;

namespace StudioTVPlayer.Providers
{
    internal class GlobalApplicationData
    {
        private const string PathName = "StudioTVPlayer";
        public static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);
        private readonly List<RundownPlayer> _rundownPlayers = new List<RundownPlayer>();
        private PlayerControllerBase[] _playerControllers = Array.Empty<PlayerControllerBase>();
        private readonly List<Recording> _recordings = new List<Recording>();

        private GlobalApplicationData()
        {
            EncoderPresets = LoadEncoderPresets();
        }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        public IReadOnlyList<RundownPlayer> RundownPlayers => _rundownPlayers;

        public IReadOnlyList<PlayerControllerBase> PlayerControllers => _playerControllers;

        public IReadOnlyList<Recording> Recordings => _recordings;

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
                rundownPlayer.PlayerStateChanged -= Player_PlayerChanged;
                rundownPlayer.ItemAdded -= Player_PlayerChanged;
                rundownPlayer.ItemRemoved -= Player_PlayerChanged;
                rundownPlayer.ItemLoaded -= Player_PlayerChanged;
                rundownPlayer.Seeked -= Player_PlayerChanged;
                rundownPlayer.Dispose();
                _rundownPlayers.Remove(rundownPlayer);
            }
            rundownPlayers.Clear();
            foreach (var playerUpdateItem in newConfiguration)
            {
                var rundownPlayer = RundownPlayers.FirstOrDefault(p => p.Configuration == playerUpdateItem.Player) ?? CreateRundownPlayer(playerUpdateItem.Player);
                if (playerUpdateItem.NeedsReinitialization)
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
            UpdatePlayers(Configuration.Current.Players.Select(p => new PlayerUpdateItem(p, true)).ToList());
            foreach (var input in InputList.Current.Inputs)
                input.Initialize();
            UpdatePlayerControllers();
        }

        public void UpdatePlayerControllers()
        {
            foreach (var playerController in _playerControllers)
            {
                playerController.ConnectionStateChanged -= PlayerController_ConnectionStateChanged;
                playerController.Dispose();
            }
            _playerControllers = Configuration.Current.PlayerControllers.Select(CreatePlayerController).ToArray();
            PlayerControllerConnectionStatusChanged?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler PlayerControllerConnectionStatusChanged;

        public bool PlayerControllersConnected => _playerControllers.All(p => p.IsConnected);

        private RundownPlayer CreateRundownPlayer(Model.Configuration.Player playerConfiguration)
        {
            var player = new RundownPlayer(playerConfiguration);
            player.PlayerStateChanged += Player_PlayerChanged;
            player.ItemAdded += Player_PlayerChanged;
            player.ItemRemoved += Player_PlayerChanged;
            player.ItemLoaded += Player_PlayerChanged;
            player.Seeked += Player_PlayerChanged;
            return player;
        }

        private void Player_PlayerChanged(object sender, EventArgs e)
        {
            var player = sender as RundownPlayer ?? throw new ArgumentException(nameof(sender));
            foreach (var playerController in PlayerControllers)
                playerController.NotifyPlayerChanged(player);
        }

        private PlayerControllerBase CreatePlayerController(Model.Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            PlayerControllerBase playerController = null;
            switch (playerControllerConfiguration)
            {
                case Model.Configuration.BlackmagicDesignAtemPlayerController bmdPlayerControllerConfiguration:
                    playerController = new BlackmagicDesignAtemPlayerController(bmdPlayerControllerConfiguration);
                    break;
                case Model.Configuration.ElgatoStreamDeckPlayerController elgatoStreamDeckPlayerControllerConfiguration:
                    playerController = new ElgatoStreamDeckPlayerController(elgatoStreamDeckPlayerControllerConfiguration);
                    break;
                default: throw new ApplicationException($"Unknown player controller ({playerControllerConfiguration})");
            }
            playerController.ConnectionStateChanged += PlayerController_ConnectionStateChanged;
            return playerController;
        }

        private void PlayerController_ConnectionStateChanged(object sender, EventArgs e) => PlayerControllerConnectionStatusChanged?.Invoke(sender, EventArgs.Empty);

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

    public class PlayerUpdateItem
    {
        public PlayerUpdateItem(Model.Configuration.Player player, bool needsReinitialization)
        {
            Player = player;
            NeedsReinitialization = needsReinitialization;
        }
        public Model.Configuration.Player Player { get; }
        public bool NeedsReinitialization { get; }
    }
}
