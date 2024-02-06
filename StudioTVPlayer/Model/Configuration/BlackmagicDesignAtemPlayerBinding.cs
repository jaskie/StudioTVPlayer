using LibAtem.Common;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerBinding : PlayerBindingBase
    {
        [XmlAttribute]
        public MixEffectBlockId Me { get; set; }
        
        [XmlAttribute]
        public VideoSource VideoSource { get; set; }

        [XmlAttribute]
        public BlackmagicDesignAtemCommand Command {  get; set; }
    }
}
