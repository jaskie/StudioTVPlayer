using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel;
using System.Linq;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerBindingBase
    {
        private readonly string _playerId;
        private readonly PlayerMethodKind _playerMethod;

        protected PlayerBindingBase(Configuration.PlayerBindingBase playerBindingConfiguration) 
        {
            _playerId = playerBindingConfiguration.PlayerId;
            _playerMethod = playerBindingConfiguration.PlayerMethod;
        }

        protected void ExecuteOnPlayer()
        {
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
