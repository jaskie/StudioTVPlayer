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

        [XmlAttribute]
        public string OutputFormat { get; set; }

        [XmlArray]
        [XmlArrayItem("Format")]
        public string[] InputFormats { get; set; }

        [XmlIgnore]
        public bool IsEmbedded { get; internal set; }
    }
}
