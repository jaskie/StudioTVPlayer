using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkOutput: OutputBase
    {
        [XmlAttribute]
        public int DeviceIndex { get; set; }

        public override TVPlayR.OutputBase GetDevice()
        {
            var info = TVPlayR.DecklinkIterator.EnumerateDevices().FirstOrDefault(i => i.Index == DeviceIndex);
            return info is null ? null :TVPlayR.DecklinkIterator.CreateOutput(info);
        }

        public override void Initialize() { }

        public override void Uninitialize() { }

    }
}
