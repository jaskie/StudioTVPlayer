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
    public class Player : INotifyPropertyChanged, IDisposable
    {
        private string _name = string.Empty;

        private TVPlayR.VideoFormat _videoFormat;
        private string _videoFormatName;
        private TVPlayR.PixelFormat _pixelFormat;
        private bool _livePreview;

        private TVPlayR.Player _player;
        private TVPlayR.PreviewSink _outputPreview;

        private List<OutputBase> _outputs = new List<OutputBase>();
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;

        [XmlAttribute]
        public string Name { get => _name; set => _name = value; }

        [XmlIgnore]
        public TVPlayR.VideoFormat VideoFormat
        {
            get => _videoFormat; 
            set
            {
                if (!Set(ref _videoFormat, value))
                    return;
                VideoFormatName = value.Name;
            }
        }

        [XmlAttribute(nameof(VideoFormat))]
        public string VideoFormatName { get => _videoFormatName; set => _videoFormatName = value; }

        [XmlAttribute]
        public TVPlayR.PixelFormat PixelFormat { get => _pixelFormat; set => Set(ref _pixelFormat, value); }

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
        public bool LivePreview { get => _livePreview; set => Set(ref _livePreview, value); }

        [XmlAttribute]
        public bool DisablePlayedItems { get => _disablePlayedItems; set => _disablePlayedItems = value; }

        [XmlAttribute]
        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => _addItemsWithAutoPlay = value; }

        [XmlIgnore]
        public bool IsInitialized => _player != null;

        [XmlIgnore]
        public int AudioChannelCount { get; } = 2;

        public void Initialize()
        {
            if (_player != null)
                throw new ApplicationException($"Player {Name} already initialized");
            _videoFormat = TVPlayR.VideoFormat.Formats.FirstOrDefault(f => f.Name == VideoFormatName);
            if (_videoFormat == null)
                return;
            _player = new TVPlayR.Player(_name, _videoFormat, PixelFormat, AudioChannelCount);
            foreach (var output in Outputs)
            {
                output.Initialize(_player);
                if (output.IsFrameClock)
                    _player.SetFrameClock(output.Output);
                _player.AddOutputSink(output.Output);
            }
            _player.AudioVolume += Player_AudioVolume;
        }


        public void SetVolume(float value)
        {
            if (_player == null)
                throw new ApplicationException($"Player {Name} not initialized");
            _player.Volume = (float)Math.Pow(10, value / 20);
        }

        public ImageSource GetPreview(int width, int height)
        {
            if (_player == null)
                throw new ApplicationException($"Player {Name} not initialized");
            if (_outputPreview is null)
            {
                _outputPreview = new TVPlayR.PreviewSink(Application.Current.Dispatcher, width, height);
                _player.AddOutputSink(_outputPreview);
            }
            return _outputPreview.PreviewSource;
        }

        public void Uninitialize()
        {
            if (_player == null)
                return;
            foreach (var o in Outputs)
            {
                _player.RemoveOutputSink(o.Output);
                o.Dispose();
            }
            if (!(_outputPreview is null))
            {
                _outputPreview.Dispose();
                _outputPreview = null;
            }
            _player.AudioVolume -= Player_AudioVolume;
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

        private bool Set<T>(ref T field, T value, [CallerMemberName] string propertyname = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value))
                return false;
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
            return true;
        }

        private void Player_AudioVolume(object sender, TVPlayR.AudioVolumeEventArgs e)
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
