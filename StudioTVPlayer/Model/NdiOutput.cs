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


        public override TVPlayR.OutputBase GetOutput()
        {
            return _outputDevice;
        }

        public override void Initialize()
        {
            _outputDevice?.Dispose();
            _outputDevice = new TVPlayR.NdiOutput(SourceName, GroupNames);
        }

        public override void Dispose()
        {
            if (_outputDevice is null)
                return;
            _outputDevice.Dispose();
            _outputDevice = null;
        }
    }
}
