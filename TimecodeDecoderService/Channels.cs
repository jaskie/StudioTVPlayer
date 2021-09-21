using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    [XmlRoot("channels")]
    public class Channels
    {
        [XmlElement("channel")]
        public Channel[] ChannelList { get; set; }
    }
}
