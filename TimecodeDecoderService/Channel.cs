using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    public enum Keyer
    {
        Internal,
        Passthrough
    }

    [XmlType("channel")]
    public class Channel
    {
        [XmlAttribute]
        public int Input { get; set; }

        [XmlAttribute]
        public int Output { get; set; }

        [XmlAttribute]
        public Keyer Keyer { get; set; }
    }
}