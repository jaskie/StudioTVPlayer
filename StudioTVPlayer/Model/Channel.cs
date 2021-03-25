using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Xml.Serialization;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    public class Channel : INotifyPropertyChanged, IDisposable
    {
        private string _name = string.Empty;

        private VideoFormat _videoFormat;
        private string _videoFormatName;
        private PixelFormat _pixelFormat;
        private int _deviceIndex;

        private TVPlayR.Channel _channelR;
        
        [XmlAttribute]
        public string Name { get => _name; set => _name = value; }

        [XmlIgnore]
        public VideoFormat VideoFormat
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
        public PixelFormat PixelFormat
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

        public bool IsInitialized => _channelR != null;

        [XmlIgnore]
        public int AudioChannelCount { get; } = 2;

        public void Initialize()
        {
            if (_channelR != null) 
                throw new ApplicationException($"Channel {Name} already initialized");
            var device = DecklinkDevice.EnumerateDevices().ElementAtOrDefault(DeviceIndex);
            _videoFormat = VideoFormat.EnumVideoFormats().FirstOrDefault(f => f.Name == VideoFormatName);
            if (device == null || _videoFormat == null)
                return;
            _channelR = new TVPlayR.Channel(_videoFormat, PixelFormat, AudioChannelCount);
            _channelR.AddOutput(device);
        }

        internal void SetVolume(double value)
        {
            if (_channelR == null)
                throw new ApplicationException($"Channel {Name} not initialized");
            _channelR.Volume = Math.Pow(10, value / 20);
        }


        public void Uninitialize()
        {
            if (_channelR == null)
                return;
            _channelR.Clear();
            _channelR.Dispose();
            _channelR = null;
        }

        public void AddOutput(DecklinkDevice device) => _channelR.AddOutput(device);

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
        private void RaisePropertyChanged([CallerMemberName]string propertyname = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyname));
        }

        public void Dispose()
        {
            if (!IsInitialized)
                return;
            try
            {
                Uninitialize();
            }
            catch
            {
                MessageBox.Show("Błąd zwalniania zasobów");
            }
            
        }

    }
}
