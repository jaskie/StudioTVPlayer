using StudioTVPlayer.Model.Args;
using System;
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
        private int _deviceIndex;

        private TVPlayR.Channel _channelR;
        private TVPlayR.PreviewDevice _previewDevice;
        private bool _livePreview;

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


        [XmlAttribute]
        public int DeviceIndex
        {
            get => _deviceIndex;
            set
            {
                if (_deviceIndex == value)
                    return;
                _deviceIndex = value;
                RaisePropertyChanged();
            }
        }

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

        public bool IsInitialized => _channelR != null;

        [XmlIgnore]
        public int AudioChannelCount { get; } = 2;

        public void Initialize()
        {
            if (_channelR != null)
                throw new ApplicationException($"Channel {Name} already initialized");
            var device = TVPlayR.DecklinkDevice.EnumerateDevices().ElementAtOrDefault(DeviceIndex);
            _videoFormat = TVPlayR.VideoFormat.EnumVideoFormats().FirstOrDefault(f => f.Name == VideoFormatName);
            if (device == null || _videoFormat == null)
                return;
            _channelR = new TVPlayR.Channel(_videoFormat, PixelFormat, AudioChannelCount);
            _channelR.AddOutput(device);
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
            if (_previewDevice is null)
            {
                _previewDevice = new TVPlayR.PreviewDevice(Application.Current.Dispatcher);
                _channelR.AddPreview(_previewDevice);
            }
            _previewDevice.CreatePreview(width, height);
            return _previewDevice.PreviewSource;
        }

        public void Uninitialize()
        {
            if (_channelR == null)
                return;
            _previewDevice?.Dispose();
            _previewDevice = null;
            _channelR.Clear();
            _channelR.Dispose();
            _channelR.AudioVolume -= ChannelR_AudioVolume;
            _channelR = null;
        }

        public void Load(RundownItem item)
        {
            Debug.Assert(item.InputFile != null);
            _channelR.Load(item.InputFile);
        }

        public void Preload(RundownItem item)
        {
            Debug.Assert(item.InputFile != null);
            _channelR.Preload(item.InputFile);
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
