using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class DecklinkInputRundownItem : RundownItemBase
    {
        [XmlAttribute]
        public int DeviceIndex { get; set; }
    }
}
