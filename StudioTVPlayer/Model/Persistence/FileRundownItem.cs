using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class FileRundownItem : RundownItemBase
    {
        [XmlAttribute]
        public string FileName { get; set; }
    }
}
