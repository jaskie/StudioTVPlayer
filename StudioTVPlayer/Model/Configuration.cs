using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;
using TVPlayR;

namespace StudioTVPlayer.Model
{
    [Serializable]
    public class Configuration
    {
        public Configuration()
        {
            _extensions = new List<string>();
            _watcherMetas = new List<WatcherMeta>();
            _channels = new List<Channel>();
            _devices = new List<Device>();
            _videoFormats = VideoFormat.EnumVideoFormats().ToList();
            _pixelFormats = Enum.GetValues(typeof(PixelFormat)).Cast<PixelFormat>().ToList();
        }

        [XmlIgnoreAttribute]
        public static Configuration Instance { get; set; } = new Configuration();

        private List<WatcherMeta> _watcherMetas;
        [XmlArray("WatcherMetas")]
        public List<WatcherMeta> WatcherMetas
        {
            get => _watcherMetas;
            set
            {
                if (_watcherMetas == value)
                    return;
                _watcherMetas = value;
            }
        }

        private Channel _channel;
        [XmlIgnoreAttribute]
        public Channel Channel
        {
            get { return _channel; }
            set
            {
                if (_channel == value)
                    return;
                _channel = value;                                 
            }
        }
        private List<Channel> _channels;
        [XmlArray("Channels")]
        public List<Channel> Channels { get => _channels; set => _channels = value; }

        [XmlIgnoreAttribute]
        private List<Device> _devices;
        [XmlIgnoreAttribute]
        public List<Device> Devices { get => _devices; set => _devices = value; }
        
        private List<VideoFormat> _videoFormats;
        [XmlIgnoreAttribute]
        public List<VideoFormat> VideoFormats { get => _videoFormats; set => _videoFormats = value; }
        
        private List<PixelFormat> _pixelFormats;
        [XmlIgnoreAttribute]
        public List<PixelFormat> PixelFormats { get => _pixelFormats; set => _pixelFormats = value; }

        private List<string> _extensions;
        [XmlArray]
        [XmlArrayItem("Extension")]
        public List<string> Extensions { get => _extensions; set => _extensions = value; }
    }
}
