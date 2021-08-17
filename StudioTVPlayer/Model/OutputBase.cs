using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class OutputBase
    {
        [XmlAttribute]
        public bool IsFrameClock { get; set; }
    }
}
