using System;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase
    {
        [XmlAttribute]
        public bool IsFrameClock { get; set; }

        public abstract void Initialize();

        public abstract TVPlayR.OutputBase GetDevice();

        public abstract void Uninitialize();
    }
}
