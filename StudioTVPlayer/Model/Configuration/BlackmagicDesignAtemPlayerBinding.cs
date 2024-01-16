using LibAtem.Common;
using LibAtem.Net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class BlackmagicDesignAtemPlayerBinding : PlayerBindingBase
    {
        [XmlAttribute]
        public MixEffectBlockId Me { get; set; }
        
        [XmlAttribute]
        public VideoSource VideoSource { get; set; }

        [XmlAttribute]
        public BlackmagicDesignAtemCommand Command {  get; set; }
    }
}
