using System.Collections.Generic;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class Configuration
    {
        [XmlArray]
        [XmlArrayItem("WatchedFolder")]
        public List<WatchedFolder> WatchedFolders { get; set; } = new List<WatchedFolder>();

        [XmlArray]
        [XmlArrayItem("Channel")]
        public List<Channel> Channels { get; set; } = new List<Channel>();

        [XmlArray]
        [XmlArrayItem("Extension")]
        public List<string> Extensions { get; set; } = new List<string>();
        
    }
}
