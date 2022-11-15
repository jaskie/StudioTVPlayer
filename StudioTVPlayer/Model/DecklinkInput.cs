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

        [XmlAttribute]
        public int DeviceIndex { get; set; }

        [XmlAttribute]
        public bool FormatAutodetection { get; set; }

        public override ImageSource Thumbnail => _preview?.PreviewSource;

        public override void Dispose()
        {
            if (_input is null)
                return;
            Uninitialize();
        }

        public override TVPlayR.InputBase Input => _input;

        public override bool Initialize()
        {
            Uninitialize();
            CurrentFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormat);
            var info = TVPlayR.DecklinkIterator.Devices.FirstOrDefault(i => i.Index == DeviceIndex);
            if (info is null || CurrentFormat is null)
                return false;
            try
            {
                _input = TVPlayR.DecklinkIterator.CreateInput(info, CurrentFormat, 2, TVPlayR.DecklinkTimecodeSource.RP188Any, true, FormatAutodetection);
                _input.FormatChanged += Input_FormatChanged;
                
                _preview = new TVPlayR.PreviewSink(Application.Current.Dispatcher, 160, 90);
                _input.AddOutputSink(_preview);
                return true;
            }
            catch
            {
                return false;
            }
        }

        public override void Uninitialize()
        {
            if (!(_preview is null))
                _input?.RemoveOutputSink(_preview);
            if (!(_input is null))
            {
                _input.FormatChanged -= Input_FormatChanged;
                _input.Dispose();
                _input = null;
            }
            _preview?.Dispose();
            _preview = null;
        }

        public override bool IsRunning => !(_input is null);

        private void Input_FormatChanged(object sender, TVPlayR.VideoFormatEventArgs e)
        {
            CurrentFormat = e.Format;
            VideoFormat = e.Format.Name;
            InputList.Current.Save();
            InputFormatChanged?.Invoke(this, e);
        }

        [XmlIgnore]
        public TVPlayR.VideoFormat CurrentFormat { get; private set; }


        public event EventHandler<TVPlayR.VideoFormatEventArgs> InputFormatChanged;

    }
}
