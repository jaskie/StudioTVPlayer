using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class DecklinkOutput : OutputBase
    {
        private int _deviceIndex;
        private TVPlayR.DecklinkKeyerType _keyer;
        private TVPlayR.TimecodeOutputSource _timecodeSource;

        [XmlAttribute]
        public int DeviceIndex { get => _deviceIndex; set => Set(ref _deviceIndex, value); }

        [XmlAttribute]
        public TVPlayR.DecklinkKeyerType Keyer { get => _keyer; set => Set(ref _keyer, value); }

        [XmlAttribute]
        public TVPlayR.TimecodeOutputSource TimecodeSource { get => _timecodeSource; set => Set(ref _timecodeSource, value); }

    }
}
