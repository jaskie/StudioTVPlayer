using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerController : PlayerControllerBase
    {
        [XmlAttribute]
        public string DeviceId { get; set; }

        [XmlAttribute]
        public string Address { get; set; }
    }
}
