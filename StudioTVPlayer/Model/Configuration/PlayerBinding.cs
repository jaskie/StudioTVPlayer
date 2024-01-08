using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class PlayerBinding: ConfigurationItemBase
    {
        private string _playerId;

        [XmlAttribute]
        public string PlayerId { get => _playerId; set => Set(ref _playerId, value); }
    }
}
