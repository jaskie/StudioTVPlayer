using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public abstract class PlayerControllerBase: ConfigurationItemBase
    {
        private string _id;
        private string _name;
        private PlayerBinding[] _playerBindings = new PlayerBinding[0];

        [XmlAttribute]
        public string Id { get => _id; set => Set(ref _id, value); }

        [XmlAttribute]
        public string Name { get => _name; set => Set(ref _name, value); }

        [XmlArray]
        public PlayerBinding[] PlayerBindings { get => _playerBindings; set => Set(ref _playerBindings, value); }

    }
}
