using System;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public abstract class PlayerControllerBase
    {
        [XmlAttribute]
        public string Id { get; set; }

        [XmlAttribute]
        public string Name { get; set; }

        [XmlArray]
        [XmlArrayItem(typeof(BlackmagicDesignAtemPlayerBinding))]
        public PlayerBindingBase[] Bindings { get; set; } = Array.Empty<PlayerBindingBase>();
    }
}
