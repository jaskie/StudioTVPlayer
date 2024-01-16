using LibAtem.Common;
using StudioTVPlayer.ViewModel;
using StudioTVPlayer.ViewModel.Main;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class BlackmagicDesignAtemPlayerBinding
    {
        private readonly MixEffectBlockId _me;
        private readonly VideoSource _videoSource;
        private readonly BlackmagicDesignAtemCommand _command;
        private readonly string _playerId;
        private readonly PlayerMethodKind _playerMethod;

        public BlackmagicDesignAtemPlayerBinding(Configuration.BlackmagicDesignAtemPlayerBinding blackmagicDesignAtemPlayerBindingConfiguration)
        {
            _me = blackmagicDesignAtemPlayerBindingConfiguration.Me;
            _videoSource = blackmagicDesignAtemPlayerBindingConfiguration.VideoSource;
            _command = blackmagicDesignAtemPlayerBindingConfiguration.Command;
            _playerId = blackmagicDesignAtemPlayerBindingConfiguration.PlayerId;
            _playerMethod = blackmagicDesignAtemPlayerBindingConfiguration.PlayerMethod;
        }

        public void AtemCommandReceived(BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
        {
            if (!(command == _command && me == _me && videoSource == _videoSource))
                return;
            if (!(MainViewModel.Instance.CurrentViewModel is PlayoutViewModel playoutViewModel))
                return;
            var playerVm = playoutViewModel.Players.FirstOrDefault(p => p.Id == _playerId);
            if (playerVm == null)
                return;
            switch (_playerMethod)
            {
                case PlayerMethodKind.Cue:
                    playerVm.OnUiThread(() => playerVm.CueCommand.Execute(null));
                    break;
                case PlayerMethodKind.LoadNext:
                    playerVm.OnUiThread(() => playerVm.LoadNextItemCommand.Execute(null));
                    break;
                case PlayerMethodKind.Play:
                    playerVm.OnUiThread(async () => await playerVm.Play());
                    break;
                case PlayerMethodKind.Pause:
                    playerVm.OnUiThread(async () => await playerVm.Pause());
                    break;
                case PlayerMethodKind.Clear:
                    playerVm.OnUiThread(() => playerVm.UnloadCommand.Execute(null));
                    break;
            }
        }

    }
}
