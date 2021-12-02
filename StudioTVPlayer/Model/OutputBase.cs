using System;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase: IDisposable
    {
        [XmlAttribute]
        public bool IsFrameClock { get; set; }

        [XmlAttribute]
        public bool TimecodeOverlay { get; set; }

        public abstract void Initialize(TVPlayR.Channel channel);

        public abstract TVPlayR.OutputBase GetOutput();

        public abstract void Dispose();
    }
}
