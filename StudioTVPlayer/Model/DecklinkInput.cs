using StudioTVPlayer.Providers;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class DecklinkInput : InputBase, IDisposable
    {
        private TVPlayR.DecklinkInput _input;
        private TVPlayR.PreviewSink _preview;
        private TVPlayR.VideoFormat _currentFormat;

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        [XmlAttribute]
        public bool FormatAutodetection { get; set; }

        public event EventHandler FormatChanged;

        public override ImageSource Thumbnail => _preview?.PreviewSource;

        public override void Dispose()
        {
            if (_input is null)
                return;
            Uninitialize();
        }

        internal override TVPlayR.InputBase TVPlayRInput => _input;

        public override bool Initialize()
        {
            Uninitialize();
            _currentFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormat);
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            if (info is null || _currentFormat is null)
                return false;
            try
            {
                _input = TVPlayR.DecklinkIterator.CreateInput(info, _currentFormat, 2, TVPlayR.DecklinkTimecodeSource.RP188Any, true, FormatAutodetection);
                _input.FormatChanged += Input_FormatChanged;
                
                _preview = new TVPlayR.PreviewSink(Application.Current.Dispatcher, 160, 90);
                _input.AddOutputSink(_preview);
                FormatChanged?.Invoke(this, EventArgs.Empty);
                return true;
            }
            catch
            {
                return false;
            }
        }

        public override void Uninitialize()
        {
            if (_preview is not null)
                _input?.RemoveOutputSink(_preview);
            if (_input is not null)
            {
                _input.FormatChanged -= Input_FormatChanged;
                _input.Dispose();
                _input = null;
            }
            _preview?.Dispose();
            _preview = null;
        }

        public override bool IsRunning => (_input is not null);

        private void Input_FormatChanged(object sender, TVPlayR.VideoFormatEventArgs e)
        {
            _currentFormat = e.Format;
            VideoFormat = e.Format.Name;
            InputList.Current.Save();
            FormatChanged?.Invoke(this, EventArgs.Empty);
        }

        internal override TVPlayR.VideoFormat CurrentFormat()
        {
            return _currentFormat;
        }

    }
}
