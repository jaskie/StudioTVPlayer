using System;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class LiveInputRundownItem : RundownItemBase
    {
        private readonly InputBase _input;

        public LiveInputRundownItem(InputBase input)
        {
            _input = input;
        }

        public override bool IsPlaying() => true;

        public override ImageSource Thumbnail => null;

        public override string Name => _input.TVPlayRInput.Name;

        public InputBase Input => _input;

        internal override TVPlayR.InputBase TVPlayRInput => _input.TVPlayRInput;

        public override bool CanSeek => false;

        public override bool IsEof => false;

        public override bool Seek(TimeSpan timeSpan)
        {
            return false;
        }

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
            SubscribeToEvents(_input.TVPlayRInput);
            return true;
        }

        public override bool Unload()
        {
            if (!base.Unload())
                return false;
            UnsubscribeFromEvents(_input.TVPlayRInput);
            return true;
        }

        protected override void Input_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            RaiseFramePlayed(e);
        }

    }
}
