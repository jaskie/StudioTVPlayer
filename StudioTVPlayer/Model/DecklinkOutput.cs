using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkOutput: OutputBase
    {
        private TVPlayR.DecklinkOutput _device;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        public override void Dispose()
        {
            if (_device is null)
                return;
            _device.Dispose();
            _device = null;
        }

        public override TVPlayR.OutputBase GetDevice() => _device;

        public override void Initialize()
        {
            var info = TVPlayR.DecklinkIterator.EnumerateDevices().FirstOrDefault(i => i.Index == DeviceIndex);
            _device = info is null ? null : TVPlayR.DecklinkIterator.CreateOutput(info);
        }


    }
}
