using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class NdiOutput: OutputBase
    {
        [XmlAttribute]
        public string SourceName { get; set; }

        [XmlAttribute]
        public string GroupName { get; set; }
    }
}
