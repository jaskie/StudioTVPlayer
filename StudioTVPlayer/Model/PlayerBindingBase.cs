using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel;
using System.Linq;
using System;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerBindingBase
    {
        protected readonly PlayerMethodKind PlayerMethod;
        private readonly Lazy<RundownPlayer> _rundownPlayer;

        protected PlayerBindingBase(Configuration.PlayerBindingBase playerBindingConfiguration) 
        {
            PlayerId = playerBindingConfiguration.PlayerId;
            PlayerMethod = playerBindingConfiguration.PlayerMethod;
            _rundownPlayer = new Lazy<RundownPlayer>(() => Providers.GlobalApplicationData.Current.RundownPlayers.FirstOrDefault(p => p.Id == PlayerId));
        }

        public readonly string PlayerId;

        protected RundownPlayer RundownPlayer => _rundownPlayer.Value;

        protected bool CanExecute => MainViewModel.Instance.CurrentViewModel is PlayoutViewModel;

        protected void ExecuteOnPlayer()
        {
            if (!CanExecute)
                return;
            var rundownPlayer = RundownPlayer;
            if (rundownPlayer is null)
                return;
            switch (PlayerMethod)
            {
                case PlayerMethodKind.Cue:
                    rundownPlayer.Cue();
                    break;
                case PlayerMethodKind.LoadNext:
                    rundownPlayer.LoadNextItem();
                    break;
                case PlayerMethodKind.Play:
                    rundownPlayer.Play();
                    break;
                case PlayerMethodKind.Pause:
                    rundownPlayer.Pause();
                    break;
                case PlayerMethodKind.Clear:
                    rundownPlayer.Clear();
                    break;
                case PlayerMethodKind.Toggle:
                    rundownPlayer.Toggle();
                    break;
            }
        }
    }
}
