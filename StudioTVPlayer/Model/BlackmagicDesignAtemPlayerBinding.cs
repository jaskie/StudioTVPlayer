using LibAtem.Common;

namespace StudioTVPlayer.Model
{
    public sealed class BlackmagicDesignAtemPlayerBinding : PlayerBindingBase
    {
        private readonly MixEffectBlockId _me;
        private readonly VideoSource _videoSource;
        private readonly BlackmagicDesignAtemCommand _command;

        public BlackmagicDesignAtemPlayerBinding(Configuration.BlackmagicDesignAtemPlayerBinding blackmagicDesignAtemPlayerBindingConfiguration)
            : base(blackmagicDesignAtemPlayerBindingConfiguration)
        {
            _me = blackmagicDesignAtemPlayerBindingConfiguration.Me;
            _videoSource = blackmagicDesignAtemPlayerBindingConfiguration.VideoSource;
            _command = blackmagicDesignAtemPlayerBindingConfiguration.Command;
        }

        public void AtemCommandReceived(BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
        {
            if (!(command == _command && me == _me && videoSource == _videoSource))
                return;
            ExecuteOnPlayer();
        }
    }
}
