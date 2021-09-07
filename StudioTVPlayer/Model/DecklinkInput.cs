using System;
using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkInput: IDisposable
    {
        private TVPlayR.DecklinkInput _input;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        [XmlAttribute]
        public string VideoFormat { get; set; }

        public void Dispose()
        {
            if (_input is null)
                return;
            _input.Dispose();
            _input = null;
        }

        public TVPlayR.DecklinkInput GetInput() => _input;

        public void Initialize()
        {
            var videoFormat = TVPlayR.VideoFormat.EnumVideoFormats().FirstOrDefault(f => f.Name == VideoFormat);
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            _input = info is null || videoFormat is null ? null : TVPlayR.DecklinkIterator.CreateInput(info, videoFormat, 2);
        }
    }
}
