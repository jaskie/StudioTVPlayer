using System;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class NdiOutput: OutputBase
    {
        private TVPlayR.NdiOutput _outputDevice;
        private TVPlayR.OverlayBase _overlay;

        [XmlAttribute]
        public string SourceName { get; set; }

        [XmlAttribute]
        public string GroupNames { get; set; }


        public override TVPlayR.OutputBase GetOutput()
        {
            return _outputDevice;
        }

        public override void Initialize(TVPlayR.Channel channel)
        {
            _outputDevice?.Dispose();
            _outputDevice = new TVPlayR.NdiOutput(SourceName, GroupNames);
            if (TimecodeOverlay)
            {
                _overlay = new TVPlayR.TimecodeOverlay(channel.VideoFormat, channel.PixelFormat);
                _outputDevice.AddOverlay(_overlay);
            }
        }

        public override void Dispose()
        {
            if (_outputDevice is null)
                return;
            (_overlay as IDisposable)?.Dispose();
            _outputDevice.Dispose();
            _outputDevice = null;
        }
    }
}
