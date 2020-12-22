using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class WatchedFolder
    {
        [XmlAttribute]
        public string Name { get; set; }
        
        [XmlAttribute]
        public string Path { get; set; }

        [XmlAttribute]
        public bool IsFiltered { get; set; }
    }
}
