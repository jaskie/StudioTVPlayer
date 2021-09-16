using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class InputBase
    {
        [XmlAttribute]
        public string VideoFormat { get; set; }

        public abstract void Initialize();
    }
}
