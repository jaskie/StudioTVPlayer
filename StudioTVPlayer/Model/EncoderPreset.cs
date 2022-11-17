using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class EncoderPreset: EncoderSettings
    {
        [XmlAttribute]
        public string PresetName { get; set; }

        [XmlElement]
        public string Description { get; set; }

        [XmlAttribute]
        public string FilenameExtension { get; set; }

        [XmlIgnore]
        public bool IsEmbedded { get; internal set; }
    }
}
