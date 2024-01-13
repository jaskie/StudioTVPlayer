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
        private string _address;
        private int _port;

        [XmlAttribute]
        public string Address { get => _address; set => Set(ref _address, value); }
        
        [XmlAttribute]
        public int Port { get => _port; set => Set(ref _port, value); }

    }
}
