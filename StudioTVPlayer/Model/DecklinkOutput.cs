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
            return TVPlayR.DecklinkOutput.EnumerateDevices().ElementAtOrDefault(DeviceIndex);
        }

        public override void Initialize() { }

        public override void Uninitialize() { }

    }
}
