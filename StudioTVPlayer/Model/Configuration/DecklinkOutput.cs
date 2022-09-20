using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class DecklinkOutput : OutputBase
    {
        private int _deviceIndex;
        private TVPlayR.DecklinkKeyer _keyer;

        [XmlAttribute]
        public int DeviceIndex { get => _deviceIndex; set => Set(ref _deviceIndex, value); }

        [XmlAttribute]
        public TVPlayR.DecklinkKeyer Keyer { get => _keyer; set => Set(ref _keyer, value); }

    }
}
