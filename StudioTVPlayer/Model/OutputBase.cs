using System;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase: IDisposable
    {
        private TVPlayR.OverlayBase _overlay;
        private readonly Configuration.OutputBase _configuration;

        public OutputBase(Configuration.OutputBase configuration)
        {
            _configuration = configuration;
        }

        public bool IsFrameClock => _configuration.IsFrameClock;

        public TVPlayR.TimecodeOverlaySource TimecodeOverlay => _configuration.TimecodeOverlay;

        public virtual void Initialize(TVPlayR.Player player) 
        {
            Output.InitializeFor(player);
            if (TimecodeOverlay != TVPlayR.TimecodeOverlaySource.None)
            {
                _overlay = new TVPlayR.TimecodeOverlay(TimecodeOverlay, player.VideoFormat, player.PixelFormat);
                Output.AddOverlay(_overlay);
            }
        }

        public abstract TVPlayR.OutputBase Output { get; }

        public virtual void Dispose()
        {
            (_overlay as IDisposable)?.Dispose();
        }
    }
}
