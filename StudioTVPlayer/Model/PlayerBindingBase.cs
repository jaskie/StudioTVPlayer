using StudioTVPlayer.ViewModel.Main;
using StudioTVPlayer.ViewModel;

namespace StudioTVPlayer.Model
{
    public abstract class PlayerBindingBase(Configuration.PlayerBindingBase playerBindingConfiguration, RundownPlayer rundownPlayer)
    {
        protected readonly PlayerMethodKind PlayerMethod = playerBindingConfiguration.PlayerMethod;
        public readonly RundownPlayer RundownPlayer = rundownPlayer;

        protected bool CanExecute => ShellViewModel.Instance.CurrentViewModel is PlayoutViewModel;

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
