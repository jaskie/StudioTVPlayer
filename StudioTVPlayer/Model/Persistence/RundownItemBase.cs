using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class RundownItemBase
    {

        [XmlAttribute]
        public bool IsAutoStart { get; set; }
    }
}