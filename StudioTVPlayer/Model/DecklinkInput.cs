using System;
using System.Linq;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkInput : InputBase, IDisposable
    {
        private ImageSource _thumbnail;
        private TVPlayR.DecklinkInput _input;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        public override ImageSource Thumbnail => _thumbnail;

        public void Dispose()
        {
            if (_input is null)
                return;
            _input.Dispose();
            _input = null;
        }

        public TVPlayR.DecklinkInput GetInput() => _input;

        public override bool Initialize()
        {
            _input?.Dispose();
            var videoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormat);
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            if (info is null || videoFormat is null)
                return false;
            try
            {
                _input = TVPlayR.DecklinkIterator.CreateInput(info, videoFormat, 2, TVPlayR.DecklinkTimecodeSource.None);
                return true;
            }
            catch
            {
                return false;
            }
        }

        public bool IsRunning => !(_input is null);
    }
}
