using StudioTVPlayer.Helpers;
using System.Collections.Generic;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class Configuration
    {
        [XmlArray]
        [XmlArrayItem("Watcher")]
        public List<Watcher> Watchers { get; set; } = new List<Watcher>();

        [XmlArray]
        [XmlArrayItem("Channel")]
        public List<Channel> Channels { get; set; } = new List<Channel>();

        [XmlArray]
        [XmlArrayItem("Extension")]
        public List<string> Extensions { get; set; } = new List<string>();
        
    }
}
