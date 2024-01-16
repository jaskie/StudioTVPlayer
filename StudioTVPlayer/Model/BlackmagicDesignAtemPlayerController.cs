using LibAtem.Commands.MixEffects;
using LibAtem.Commands.MixEffects.Transition;
using LibAtem.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public sealed class BlackmagicDesignAtemPlayerController : PlayerControllerBase
    {
        private bool _disposed;
        private readonly string _address;
        private readonly LibAtem.Net.AtemClient _atemClient;
        private readonly BlackmagicDesignAtemPlayerBinding[] _bindings;

        public BlackmagicDesignAtemPlayerController(Configuration.BlackmagicDesignAtemPlayerController bmdPlayerControllerConfiguration)
        {
            _address = bmdPlayerControllerConfiguration.Address;
            _atemClient = BlackmagicDesignAtemDevices.GetDevice(_address);
            _atemClient.OnReceive += OnReceive;
            _bindings = bmdPlayerControllerConfiguration.Bindings.Select(CreateBinding).ToArray();
        }

        private BlackmagicDesignAtemPlayerBinding CreateBinding(Configuration.PlayerBindingBase playerBindingConfiguration)
        {
            var blackmagicDesignAtemPlayerBindingConfiguration = playerBindingConfiguration as Configuration.BlackmagicDesignAtemPlayerBinding ?? throw new ArgumentException(nameof(playerBindingConfiguration));
            return new BlackmagicDesignAtemPlayerBinding(blackmagicDesignAtemPlayerBindingConfiguration);
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

        public override void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            BlackmagicDesignAtemDevices.ReleaseDevice(_address);
        }
    }
}
