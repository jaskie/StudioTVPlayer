using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class Rundown : Helpers.IPersistable
    {

        [XmlElement(typeof(FileRundownItem))]
        [XmlElement(typeof(DecklinkInputRundownItem))]
        public RundownItemBase[] RundownItems { get; set; }
    }
}
