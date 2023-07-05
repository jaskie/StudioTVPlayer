using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class DecklinkInputRundownItem : RundownItemBase
    {
        [XmlAttribute]
        public int DeviceIndex { get; set; }
    }
}
