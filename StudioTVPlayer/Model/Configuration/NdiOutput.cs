using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class NdiOutput : OutputBase
    {
        private string _sourceName;
        private string _groupNames;

        [XmlAttribute]
        public string SourceName { get => _sourceName; set => Set(ref _sourceName, value); }

        [XmlAttribute]
        public string GroupNames { get => _groupNames; set => Set(ref _groupNames, value); }

    }
}
