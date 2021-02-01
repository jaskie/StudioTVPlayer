using System;
using System.ComponentModel;
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

        public void Initialize()
        {
            if (_channelR != null) 
                throw new ApplicationException($"Channel {Name} already initialized");
            var device = DecklinkDevice.EnumerateDevices().ElementAtOrDefault(DeviceIndex);
            var format = VideoFormat.EnumVideoFormats().FirstOrDefault(f => f.Name == VideoFormatName);
            if (device == null || format == null)
                return;
            _channelR = new TVPlayR.Channel(format, PixelFormat, 16);
            _channelR.AddOutput(device);
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
        public void Load(InputFile inputFile) => _channelR.Load(inputFile);
        public void Clear() => _channelR.Clear();

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
