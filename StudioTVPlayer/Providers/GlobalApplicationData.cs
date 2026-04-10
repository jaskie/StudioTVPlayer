using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace StudioTVPlayer.Providers
{
    internal class GlobalApplicationData
    {
        private const string PathName = "StudioTVPlayer";
        public static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);
        private readonly List<RundownPlayer> _rundownPlayers = [];
        private PlayerControllerBase[] _playerControllers = [];
        private GlobalApplicationData() { }

        public static GlobalApplicationData Current { get; } = new GlobalApplicationData();

        public IReadOnlyList<RundownPlayer> RundownPlayers => _rundownPlayers;

        public IReadOnlyList<PlayerControllerBase> PlayerControllers => _playerControllers;

        public IReadOnlyList<EncoderPreset> EncoderPresets { get; private set; }

        public void Shutdown()
        {
            MediaVerifier.Current.Dispose();
            RecordingStore.Current.Dispose();
            foreach (var player in RundownPlayers)
                player.Dispose();
            foreach (var input in InputList.Current.Inputs)
                input.Dispose();
            foreach (var playerController in PlayerControllers)
                playerController.Dispose();
            RecordingScheduler.Current.Shutdown();
        }

        public void UpdatePlayers(List<PlayerUpdateItem> newConfiguration)
        {
            var rundownPlayers = RundownPlayers.Where(p => !newConfiguration.Select(x => x.Player).Contains(p.Configuration)).ToList();
            foreach (var rundownPlayer in rundownPlayers)
            {
                rundownPlayer.PlayerStateChanged -= Player_PlayerChanged;
                rundownPlayer.ItemAdded -= Player_PlayerChanged;
                rundownPlayer.ItemRemoved -= Player_PlayerChanged;
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
            UpdatePlayers([.. Configuration.Current.Players.Select(p => new PlayerUpdateItem(p, true))]);
            foreach (var input in InputList.Current.Inputs)
                input.Initialize();
            UpdatePlayerControllers();
            LoadRundownsFromCommandLine();
            RecordingScheduler.Current.Initialize();
        }

        public void UpdatePlayerControllers()
        {
            foreach (var playerController in _playerControllers)
            {
                playerController.ConnectionStateChanged -= PlayerController_ConnectionStateChanged;
                playerController.Dispose();
            }
            _playerControllers = [.. Configuration.Current.PlayerControllers.Select(CreatePlayerController)];
            PlayerControllerConnectionStatusChanged?.Invoke(this, EventArgs.Empty);
            PlayerControllersModified?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler PlayerControllerConnectionStatusChanged;

        public event EventHandler PlayerControllersModified;

        public bool AllPlayerControllersConnected => _playerControllers.All(p => p.IsConnected);

        private RundownPlayer CreateRundownPlayer(Model.Configuration.Player playerConfiguration)
        {
            var player = new RundownPlayer(playerConfiguration);
            player.PlayerStateChanged += Player_PlayerChanged;
            player.ItemAdded += Player_PlayerChanged;
            player.ItemRemoved += Player_PlayerChanged;
            player.Seeked += Player_PlayerChanged;
            return player;
        }

        private void LoadRundownsFromCommandLine()
        {
            foreach (var player in _rundownPlayers)
            {
                var rundownToLoad = CommandLineProcessor.Instance.RundownToLoad(player.Name);
                if (!string.IsNullOrEmpty(rundownToLoad))
                {
                    player.LoadRundown(rundownToLoad);
                    player.LoadNextItem();
                }
            }
        }

        private void Player_PlayerChanged(object sender, EventArgs e)
        {
            var player = sender as RundownPlayer ?? throw new ArgumentException($"{nameof(RundownPlayer)} expected, {sender?.GetType()} got.");
            foreach (var playerController in PlayerControllers)
                playerController.NotifyPlayerChanged(player);
        }

        private PlayerControllerBase CreatePlayerController(Model.Configuration.PlayerControllerBase playerControllerConfiguration)
        {
            PlayerControllerBase playerController = null;
            playerController = playerControllerConfiguration switch
            {
                Model.Configuration.BlackmagicDesignAtemPlayerController bmdPlayerControllerConfiguration => new BlackmagicDesignAtemPlayerController(bmdPlayerControllerConfiguration, RundownPlayers),
                Model.Configuration.ElgatoStreamDeckPlayerController elgatoStreamDeckPlayerControllerConfiguration => new ElgatoStreamDeckPlayerController(elgatoStreamDeckPlayerControllerConfiguration, RundownPlayers),
                _ => throw new ApplicationException($"Unknown player controller ({playerControllerConfiguration})"),
            };
            playerController.ConnectionStateChanged += PlayerController_ConnectionStateChanged;
            return playerController;
        }

        private void PlayerController_ConnectionStateChanged(object sender, EventArgs e) => PlayerControllerConnectionStatusChanged?.Invoke(sender, EventArgs.Empty);
    }

    public class PlayerUpdateItem(Model.Configuration.Player player, bool needsReinitialization)
    {
        public Model.Configuration.Player Player { get; } = player;
        public bool NeedsReinitialization { get; } = needsReinitialization;
    }
}
