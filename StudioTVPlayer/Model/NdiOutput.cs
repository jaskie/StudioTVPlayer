using System;
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

        public override TVPlayR.OutputBase Output => _outputDevice;

        public override void Initialize(TVPlayR.Channel channel)
        {
            _outputDevice?.Dispose();
            _outputDevice = new TVPlayR.NdiOutput(SourceName, GroupNames);
            base.Initialize(channel);
        }

        public override void Dispose()
        {
            if (_outputDevice is null)
                return;
            base.Dispose();
            _outputDevice.Dispose();
            _outputDevice = null;
        }
    }
}
