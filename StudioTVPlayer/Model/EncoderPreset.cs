using System.Collections.Generic;
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

        /// <summary>
        /// Formats for which the preset can be used.
        /// If null, there is no input format restriction.
        /// </summary>
        [XmlArray]
        [XmlArrayItem("Format")]
        public string[] InputFormats { get; set; }
    }

    public sealed class EncoderPresets
    {
        public static EncoderPresets Instance { get; } = new EncoderPresets();
        private EncoderPresets() 
        {
            _presets = Load();
        }

        private EncoderPreset[] _presets;

        public IReadOnlyCollection<EncoderPreset> Presets => _presets;

        private EncoderPreset[] Load()
        {
            var assembly = System.Reflection.Assembly.GetExecutingAssembly();
            var resourceStream = assembly.GetManifestResourceStream($"{assembly.GetName().Name}.Resources.EmbeddedPresets.xml");
            var serializer = new System.Xml.Serialization.XmlSerializer(typeof(EncoderPreset[]), new System.Xml.Serialization.XmlRootAttribute("PresetList"));
            var presets = (EncoderPreset[])serializer.Deserialize(resourceStream);
            return presets;
        }

    }
}
