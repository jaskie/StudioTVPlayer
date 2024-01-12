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
        private string _deviceAddress;

        [XmlAttribute]
        public string DeviceId { get => _deviceAddress; set => Set(ref _deviceAddress, value); }

    }
}
