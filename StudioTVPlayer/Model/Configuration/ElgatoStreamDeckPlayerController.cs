using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class ElgatoStreamDeckPlayerController : PlayerControllerBase
    {
        [XmlAttribute]
        public string Path { get; set; }
    }
}
