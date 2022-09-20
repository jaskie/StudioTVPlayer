using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class OutputBase : ConfigurationItemBase
    {
        private TVPlayR.TimecodeOverlaySource _timecodeOverlay;
        private bool _isFrameClock;

        [XmlAttribute]
        public bool IsFrameClock { get => _isFrameClock; set => Set(ref _isFrameClock, value); }

        [XmlAttribute]
        public TVPlayR.TimecodeOverlaySource TimecodeOverlay { get => _timecodeOverlay; set => Set(ref _timecodeOverlay, value); }
    }
}
