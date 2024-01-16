using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public abstract class PlayerBindingBase
    {
        [XmlAttribute]
        public string PlayerId { get; set; }

        [XmlAttribute]
        public PlayerMethodKind PlayerMethod { get; set; }

    }
}
