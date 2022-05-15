using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkOutput: OutputBase
    {
        private TVPlayR.DecklinkOutput _output;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        [XmlAttribute]
        public TVPlayR.DecklinkKeyer Keyer { get; set; }

        public override void Dispose()
        {
            if (_output is null)
                return;
            base.Dispose();
            _output.Dispose();
            _output = null;
        }

        public override TVPlayR.OutputBase Output => _output;

        public override void Initialize(TVPlayR.Player player)
        {
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            _output = info is null ? null : TVPlayR.DecklinkIterator.CreateOutput(info, Keyer);
            base.Initialize(player);
        }


    }
}
