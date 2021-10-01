using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public class LiveInputRundownItem : RundownItemBase
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

        public override void Play()
        {
        }

        public override bool Preload(int audioChannelCount)
        {
            return true;
        }

        public override void Unload()
        {
        }

        internal override void Pause()
        {
        }
    }
}
