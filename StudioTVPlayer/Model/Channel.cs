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

        private TVPlayR.Player _player;
        private TVPlayR.OutputPreview _outputPreview;

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
        [XmlArrayItem(typeof(FFOutput))]
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
        public bool IsInitialized => _player != null;

        [XmlIgnore]
        public int AudioChannelCount { get; } = 2;

        public void Initialize()
        {
            if (_player != null)
                throw new ApplicationException($"Channel {Name} already initialized");
            _videoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormatName);
            if (_videoFormat == null)
                return;
            _player = new TVPlayR.Player(_name, _videoFormat, PixelFormat, AudioChannelCount);
            foreach (var output in Outputs)
            {
                output.Initialize(_player);
                _player.AddOutput(output.Output, output.IsFrameClock);
            }
            _player.AudioVolume += ChannelR_AudioVolume;
        }


        public void SetVolume(double value)
        {
            if (_player == null)
                throw new ApplicationException($"Channel {Name} not initialized");
            _player.Volume = Math.Pow(10, value / 20);
        }

        public ImageSource GetPreview(int width, int height)
        {
            if (_player == null)
                throw new ApplicationException($"Channel {Name} not initialized");
            if (_outputPreview is null)
            {
                _outputPreview = new TVPlayR.OutputPreview(Application.Current.Dispatcher, width, height);
                _player.AddOutput(_outputPreview, false);
            }
            return _outputPreview.PreviewSource;
        }

        public void Uninitialize()
        {
            if (_player == null)
                return;
            foreach (var o in Outputs)
            {
                _player.RemoveOutput(o.Output);
                o.Dispose();
            }
            if (!(_outputPreview is null))
            {
                _outputPreview.Dispose();
                _outputPreview = null;
            }
            _player.AudioVolume -= ChannelR_AudioVolume;
            _player.Clear();
            _player.Dispose();
            _player = null;
        }

        public void Load(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.Load(item);
        }

        public void Preload(TVPlayR.InputBase item)
        {
            Debug.Assert(item != null);
            _player.Preload(item);
        }

        public void Clear()
        {
            _player.Clear();
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
