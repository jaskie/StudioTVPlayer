using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class ElgatoStreamDeckPlayerBinding : PlayerBindingBase
    {
        [XmlAttribute]
        public int Key { get; set; } = 1;
    }
}
