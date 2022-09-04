using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class EncoderSettings
    {

        [XmlAttribute]
        public string VideoCodec { get; set; }

        [XmlAttribute]
        public string AudioCodec { get; set; }

        [XmlAttribute]
        public int VideoBitrate { get; set; }

        [XmlAttribute]
        public int AudioBitrate { get; set; }

        [XmlAttribute]
        public string VideoFilter { get; set; }

        [XmlAttribute]
        public string OutputMetadata { get; set; }

        [XmlAttribute]
        public string AudioMetadata { get; set; }

        [XmlAttribute]
        public string VideoMetadata { get; set; }

        [XmlAttribute]
        public string Options { get; set; }

        [XmlAttribute]
        public int VideoStreamId { get; set; }

        [XmlAttribute]
        public int AudioStreamId { get; set; }
    }
}
