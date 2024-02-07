using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerBindingBase
    {
        protected readonly PlayerMethodKind PlayerMethod;

        protected PlayerBindingBase(Configuration.PlayerBindingBase playerBindingConfiguration, RundownPlayer rundownPlayer) 
        {
            PlayerMethod = playerBindingConfiguration.PlayerMethod;
            RundownPlayer = rundownPlayer;
        }

        public readonly RundownPlayer RundownPlayer;

        protected bool CanExecute => MainViewModel.Instance.CurrentViewModel is PlayoutViewModel;

        protected void ExecuteOnPlayer()
        {
            if (!CanExecute)
                return;
            if (RundownPlayer is null)
                return;
            switch (PlayerMethod)
            {
                case PlayerMethodKind.Cue:
                    RundownPlayer.Cue();
                    break;
                case PlayerMethodKind.LoadNext:
                    RundownPlayer.LoadNextItem();
                    break;
                case PlayerMethodKind.Play:
                    RundownPlayer.Play();
                    break;
                case PlayerMethodKind.Pause:
                    RundownPlayer.Pause();
                    break;
                case PlayerMethodKind.Clear:
                    RundownPlayer.Clear();
                    break;
                case PlayerMethodKind.Toggle:
                    RundownPlayer.Toggle();
                    break;
            }
        }
    }
}
