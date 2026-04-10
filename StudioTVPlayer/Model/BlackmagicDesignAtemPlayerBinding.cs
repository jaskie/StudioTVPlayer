using LibAtem.Common;

namespace StudioTVPlayer.Model
{
    public sealed class BlackmagicDesignAtemPlayerBinding(Configuration.BlackmagicDesignAtemPlayerBinding blackmagicDesignAtemPlayerBindingConfiguration, RundownPlayer rundownPlayer) : PlayerBindingBase(blackmagicDesignAtemPlayerBindingConfiguration, rundownPlayer)
    {
        private readonly MixEffectBlockId _me = blackmagicDesignAtemPlayerBindingConfiguration.Me;
        private readonly VideoSource _videoSource = blackmagicDesignAtemPlayerBindingConfiguration.VideoSource;
        private readonly BlackmagicDesignAtemCommand _command = blackmagicDesignAtemPlayerBindingConfiguration.Command;

        public void AtemCommandReceived(BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
        {
            if (!(command == _command && me == _me && videoSource == _videoSource))
                return;
            ExecuteOnPlayer();
        }
    }
}
