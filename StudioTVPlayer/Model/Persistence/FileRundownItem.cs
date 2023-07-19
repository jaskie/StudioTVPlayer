using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class FileRundownItem : RundownItemBase
    {
        [XmlAttribute]
        public string FileName { get; set; }
    }
}
