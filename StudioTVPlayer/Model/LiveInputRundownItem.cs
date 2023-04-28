using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class LiveInputRundownItem : RundownItemBase
    {
        TVPlayR.InputBase _input;

        public LiveInputRundownItem(InputBase input)
        {
            _input = input.Input;
        }

        public override bool IsPlaying => true;

        public override TVPlayR.InputBase Input => _input;

        public override ImageSource Thumbnail => null;

        public override string Name => _input.Name;

        public override bool CanSeek => false;

        public override void Play()
        {
        }

        public override void Pause()
        {
        }

        public override bool Prepare(int audioChannelCount)
        {
            if (!base.Prepare(audioChannelCount))
                return false;
            SubscribeToEvents();
            return true;
        }

        public override bool Unload()
        {
            if (!base.Unload())
                return false;
            UnsubscribeFromEvents();
            return true;
        }

        protected override void Input_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            RaiseFramePlayed(e);
        }
    }
}
