using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public abstract class PlayerControllerBase: ConfigurationItemBase
    {
        private PlayerBinding[] _playerBindings = new PlayerBinding[0];

        [XmlArray]
        public PlayerBinding[] PlayerBindings { get => _playerBindings; set => Set(ref _playerBindings, value); }
    }
}
