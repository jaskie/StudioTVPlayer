using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerController : PlayerControllerBase
    {
        [XmlAttribute]
        public string Address { get; set; }
    }
}
