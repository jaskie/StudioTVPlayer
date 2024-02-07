using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class FFOutput : OutputBase
    {
        private string _url;
        private EncoderSettings _encoderSettings;

        [XmlAttribute]
        public string Url { get => _url; set => Set(ref _url, value); }

        [XmlElement]
        public EncoderSettings EncoderSettings { get => _encoderSettings; set => Set(ref _encoderSettings, value); }
    }
}
