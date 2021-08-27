using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class NdiOutput: OutputBase
    {
        private TVPlayR.NdiOutput _outputDevice;

        [XmlAttribute]
        public string SourceName { get; set; }

        [XmlAttribute]
        public string GroupNames { get; set; }

        public override void Uninitialize()
        {
            _outputDevice?.Dispose();
            _outputDevice = null;
        }

        public override TVPlayR.OutputBase GetDevice()
        {
            return _outputDevice;
        }

        public override void Initialize()
        {
            _outputDevice?.Dispose();
            _outputDevice = new TVPlayR.NdiOutput(SourceName, GroupNames);
        }
    }
}
