using System;
using System.Windows.Media;

namespace StudioTVPlayer.Model
{
    public sealed class LiveInputRundownItem(InputBase input) : RundownItemBase
    {
        public override bool IsPlaying() => true;

        public override ImageSource Thumbnail => null;

        public override string Name => input.Name;

        public InputBase Input => input;

        internal override TVPlayR.InputBase TVPlayRInput => input.TVPlayRInput;

        public override bool CanSeek() => false;

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
            SubscribeToEvents(input.TVPlayRInput);
            return true;
        }

        public override bool Unload()
        {
            if (!base.Unload())
                return false;
            UnsubscribeFromEvents(input.TVPlayRInput);
            return true;
        }

        protected override void Input_FramePlayed(object sender, TVPlayR.TimeEventArgs e)
        {
            RaiseFramePlayed(e);
        }

    }
}
