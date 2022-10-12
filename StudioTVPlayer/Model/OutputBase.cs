using System;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase: IDisposable
    {
        private TVPlayR.OverlayBase _overlay;

        public OutputBase(Configuration.OutputBase configuration)
        {
            Configuration = configuration;
        }

        public Configuration.OutputBase Configuration { get; }

        public bool IsFrameClock => Configuration.IsFrameClock;

        public TVPlayR.TimecodeOutputSource TimecodeOverlay => Configuration.TimecodeOverlay;

        public virtual void Initialize(TVPlayR.Player player) 
        {
            Output.InitializeFor(player);
            if (TimecodeOverlay != TVPlayR.TimecodeOutputSource.None)
            {
                _overlay = new TVPlayR.TimecodeOverlay(TimecodeOverlay, player.VideoFormat, player.PixelFormat);
                Output.AddOverlay(_overlay);
            }
        }

        public virtual void UnInitialize()
        {
            (_overlay as IDisposable)?.Dispose();
            _overlay = null;
        }

        public abstract TVPlayR.OutputBase Output { get; }

        public virtual void Dispose()
        {
            UnInitialize();
        }
    }
}
