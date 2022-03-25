using System;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase: IDisposable
    {
        private TVPlayR.OverlayBase _overlay;
        
        [XmlAttribute]
        public bool IsFrameClock { get; set; }

        [XmlAttribute]
        public bool TimecodeOverlay { get; set; }

        public virtual void Initialize(TVPlayR.Player player) 
        {
            if (TimecodeOverlay)
            {
                _overlay = new TVPlayR.TimecodeOverlay(player.VideoFormat, player.PixelFormat);
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
