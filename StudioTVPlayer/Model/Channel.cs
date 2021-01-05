using System;
using System.ComponentModel;
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
        private int _videoFormatId;
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
                VideoFormatId = value.Id;
                RaisePropertyChanged();
            }
        }

        [XmlAttribute]
        public int VideoFormatId { get => _videoFormatId; set => _videoFormatId = value; }

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

        public void Initialize(int videoFormat, PixelFormat pixelFormat)
        {
            _channelR = new TVPlayR.Channel(videoFormat, pixelFormat, 16);
        }

        public void Uninitialize()
        {

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
            if (_channelR == null)
                return;
            try
            {
                Clear();
                _channelR.Dispose();
                _channelR = null;
            }
            catch
            {
                MessageBox.Show("Błąd zwalniania zasobów");
            }
            
        }
    }
}
