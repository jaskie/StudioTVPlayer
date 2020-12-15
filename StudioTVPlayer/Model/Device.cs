using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    [Serializable]
    public class Device
    {              
        private int _id;
        [XmlElement("ID")]
        public int ID { get => _id; set => _id = value; }
       
        private string _name;
        [XmlElement("Name")]
        public string Name { get => _name; set => _name = value; }       
    }
}
