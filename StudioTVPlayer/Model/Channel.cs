using StudioTVPlayer.Model.Args;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class Channel : INotifyPropertyChanged, IDisposable
    {
        private string _name = string.Empty;

        private TVPlayR.VideoFormat _videoFormat;
        private string _videoFormatName;
        private TVPlayR.PixelFormat _pixelFormat;
        private bool _livePreview;

        private TVPlayR.Channel _channelR;
        private TVPlayR.PreviewOutput _previewOutput;

        private List<OutputBase> _outputs = new List<OutputBase>();

        [XmlAttribute]
        public string Name { get => _name; set => _name = value; }

        [XmlIgnore]
        public TVPlayR.VideoFormat VideoFormat
        {
            get => _videoFormat;
            set
            {
                if (_videoFormat == value)
                    return;

                _videoFormat = value;
                VideoFormatName = value.Name;
                RaisePropertyChanged();
            }
        }

        [XmlAttribute(nameof(VideoFormat))]
        public string VideoFormatName { get => _videoFormatName; set => _videoFormatName = value; }

        [XmlAttribute]
        public TVPlayR.PixelFormat PixelFormat
        {
            get => _pixelFormat;
            set
            {
                if (_pixelFormat == value)
                    return;
                _pixelFormat = value;
                RaisePropertyChanged();
            }
        }

        [XmlArray]
        [XmlArrayItem(typeof(DecklinkOutput))]
        [XmlArrayItem(typeof(NdiOutput))]
        [XmlArrayItem(typeof(StreamOutput))]
        public OutputBase[] Outputs
        {
            get => _outputs.ToArray();
            set
            {
                foreach (var output in _outputs)
                    output.Dispose();
                _outputs.Clear();
                if (value is null)
                    return;
                _outputs.AddRange(value);
            }
        }

        [XmlAttribute]
        public bool LivePreview
        {
            get => _livePreview;
            set
            {
               if (_livePreview == value)
                    return;
                _livePreview = value;
                RaisePropertyChanged();
            }
        }

        [XmlIgnore]
        public bool IsInitialized => _channelR != null;

        [XmlIgnore]
        public int AudioChannelCount { get; } = 2;

        public void Initialize()
        {
            if (_channelR != null)
                throw new ApplicationException($"Channel {Name} already initialized");
            _videoFormat = TVPlayR.VideoFormat.EnumVideoFormats().FirstOrDefault(f => f.Name == VideoFormatName);
            if (_videoFormat == null)
                return;
            _channelR = new TVPlayR.Channel(_name, _videoFormat, PixelFormat, AudioChannelCount);
            foreach (var output in Outputs)
            {
                output.Initialize();
                _channelR.AddOutput(output.GetOutput(), output.IsFrameClock);
            }
            _channelR.AudioVolume += ChannelR_AudioVolume;
        }


        public void SetVolume(double value)
        {
            if (_channelR == null)
                throw new ApplicationException($"Channel {Name} not initialized");
            _channelR.Volume = Math.Pow(10, value / 20);
        }

        public ImageSource GetPreview(int width, int height)
        {
            if (_channelR == null)
                throw new ApplicationException($"Channel {Name} not initialized");
            if (_previewOutput is null)
            {
                _previewOutput = new TVPlayR.PreviewOutput(Application.Current.Dispatcher, width, height);
                _channelR.AddOutput(_previewOutput, false);
            }
            return _previewOutput.PreviewSource;
        }

        public void Uninitialize()
        {
            if (_channelR == null)
                return;
            _channelR.Clear();
            _channelR.Dispose();
            _previewOutput?.Dispose();
            _previewOutput = null;
            _channelR.AudioVolume -= ChannelR_AudioVolume;
            _channelR = null;
        }

        public void Load(RundownItem item)
        {
            Debug.Assert(item.FileInput != null);
            _channelR.Load(item.FileInput);
        }

        public void Preload(RundownItem item)
        {
            Debug.Assert(item.FileInput != null);
            _channelR.Preload(item.FileInput);
        }

        public void Clear()
        {
            _channelR.Clear();
        }

        public event PropertyChangedEventHandler PropertyChanged;
        public event EventHandler<AudioVolumeEventArgs> AudioVolume;
        private void RaisePropertyChanged([CallerMemberName] string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        private void ChannelR_AudioVolume(object sender, TVPlayR.AudioVolumeEventArgs e)
        {
            AudioVolume?.Invoke(this, new AudioVolumeEventArgs(e.AudioVolume));
        }

        public void Dispose()
        {
            if (!IsInitialized)
                return;
            Uninitialize();
        }

    }
}
