using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkOutput: OutputBase
    {
        private TVPlayR.DecklinkOutput _output;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        public override void Dispose()
        {
            if (_output is null)
                return;
            _output.Dispose();
            _output = null;
        }

        public override TVPlayR.OutputBase GetOutput() => _output;

        public override void Initialize()
        {
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            _output = info is null ? null : TVPlayR.DecklinkIterator.CreateOutput(info);
        }


    }
}
