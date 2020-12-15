using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Xml.Serialization;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    [Serializable]
    public class Channel : INotifyPropertyChanged, IDisposable
    {
        private int _id;
        [XmlElement("ID")]
        public int ID { get => _id; set => _id = value; }
      
        private TVPlayR.Channel _channelR;
        
        private string _name;
        [XmlElement("Name")]
        public string Name { get => _name; set => _name = value; }

        private VideoFormat _videoFormat;
        [XmlIgnoreAttribute]
        public VideoFormat VideoFormat
        {
            get => _videoFormat;
            set
            {
                if (_videoFormat == value)
                    return;

                _videoFormat = value;
                VideoFormatId = value.Id;
                RaisePropertyRaised();
            }
        }

        private int _videoFormatId;
        [XmlElement("VideoFormatID")]
        public int VideoFormatId { get => _videoFormatId; set => _videoFormatId = value; }

        private PixelFormat _pixelFormat;
        [XmlElement("PixelFormat")]
        public PixelFormat PixelFormat
        {
            get => _pixelFormat;
            set
            {
                if (_pixelFormat == value)
                    return;

                _pixelFormat = value;
                RaisePropertyRaised();
            }
        }

        private int _audioChannelsCount;
        [XmlElement("AudioChannels")]
        public int AudioChannelsCount
        {
            get { return _audioChannelsCount; }
            set
            {
                if (_audioChannelsCount == value)
                    return;

                _audioChannelsCount = value;
                RaisePropertyRaised();
            }
        }

        private Device _device;     
        [XmlElement("Device")]
        public Device Device
        {
            get => _device;
            set
            {
                if (_device == value)
                    return;

                _device = value;
                RaisePropertyRaised();
            }
        }

        public void Init(int videoFormat, PixelFormat pixelFormat, int audioChannelCount) => _channelR = new TVPlayR.Channel(videoFormat, pixelFormat, audioChannelCount);
        public void AddOutput(TVPlayR.DecklinkDevice device) => _channelR.AddOutput(device);
        public void Load(InputFile inputFile) => _channelR.Load(inputFile);
        public void Clear() => _channelR.Clear();

        public event PropertyChangedEventHandler PropertyChanged;
        private void RaisePropertyRaised([CallerMemberName]string propertyname = null)
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
