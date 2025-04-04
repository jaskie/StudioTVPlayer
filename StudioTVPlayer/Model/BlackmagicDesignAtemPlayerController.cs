using LibAtem.Commands.MixEffects;
using LibAtem.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace StudioTVPlayer.Model
{
    public sealed class BlackmagicDesignAtemPlayerController : PlayerControllerBase
    {
        private bool _disposed;
        private readonly string _address;
        private readonly LibAtem.Net.AtemClient _atemClient;
        private readonly BlackmagicDesignAtemPlayerBinding[] _bindings;

        public BlackmagicDesignAtemPlayerController(Configuration.BlackmagicDesignAtemPlayerController bmdPlayerControllerConfiguration, IReadOnlyList<RundownPlayer> rundownPlayers)
            : base(bmdPlayerControllerConfiguration)
        {
            _address = bmdPlayerControllerConfiguration.Address;
            _atemClient = BlackmagicDesignAtemDevices.GetDevice(_address);
            _atemClient.OnConnection += OnConnection;
            _atemClient.OnDisconnect += OnDisconnect;
            _atemClient.OnReceive += OnReceive;
            IsConnected = BlackmagicDesignAtemDevices.IsConnected(_address);
            _bindings = bmdPlayerControllerConfiguration.Bindings.Select(bindingConfiguration => CreateBinding(bindingConfiguration, rundownPlayers.FirstOrDefault(p => p.Id == bindingConfiguration.PlayerId))).ToArray();
        }

        public override void NotifyPlayerChanged(RundownPlayer player) { }

        private BlackmagicDesignAtemPlayerBinding CreateBinding(Configuration.PlayerBindingBase playerBindingConfiguration, RundownPlayer rundownPlayer)
        {
            Debug.Assert(rundownPlayer != null);
            var blackmagicDesignAtemPlayerBindingConfiguration = playerBindingConfiguration as Configuration.BlackmagicDesignAtemPlayerBinding ?? throw new ArgumentException(nameof(playerBindingConfiguration));
            return new BlackmagicDesignAtemPlayerBinding(blackmagicDesignAtemPlayerBindingConfiguration, rundownPlayer);
        }

        private void OnReceive(object sender, IReadOnlyList<LibAtem.Commands.ICommand> commands)
        {
            foreach (var cmd in commands)
            {
                switch (cmd)
                {
                    case ProgramInputGetCommand prgI:
                        NotifyBindings(BlackmagicDesignAtemCommand.PrgI, prgI.Index, prgI.Source);
                        break;
                    case PreviewInputGetCommand prvI:
                        NotifyBindings(BlackmagicDesignAtemCommand.PrvI, prvI.Index, prvI.Source);
                        break;
                    default:
                        break;
                }
            }
        }

        private void NotifyBindings(BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
        {
            foreach (var binding in _bindings)
                binding.AtemCommandReceived(command, me, videoSource);
        }

        private void OnConnection(object _)
        {
            NotifyConnectionStateChanged(true);
        }

        private void OnDisconnect(object _)
        {
            NotifyConnectionStateChanged(false);
        }

        public override void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            _atemClient.OnReceive -= OnReceive;
            _atemClient.OnConnection -= OnConnection;
            _atemClient.OnDisconnect -= OnDisconnect;
            BlackmagicDesignAtemDevices.ReleaseDevice(_address);
        }
    }
}
