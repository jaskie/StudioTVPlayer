using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public class Player : ConfigurationItemBase
    {
        private string _name;
        private string _videoFormat;
        private TVPlayR.PixelFormat _pixelFormat;
        private bool _livePreview;
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;

        [XmlAttribute]
        public string Name { get => _name; set => Set(ref _name, value); }

        [XmlAttribute]
        public string VideoFormat { get => _videoFormat; set => Set(ref _videoFormat, value); }

        [XmlAttribute]
        public TVPlayR.PixelFormat PixelFormat { get => _pixelFormat; set => Set(ref _pixelFormat, value); }

        [XmlAttribute]
        public bool LivePreview { get => _livePreview; set => Set(ref _livePreview, value); }

        [XmlAttribute]
        public bool DisablePlayedItems { get => _disablePlayedItems; set => Set(ref _disablePlayedItems, value); }

        [XmlAttribute]
        public bool AddItemsWithAutoPlay { get => _addItemsWithAutoPlay; set => Set(ref _addItemsWithAutoPlay, value); }

        [XmlArray]
        [XmlArrayItem(typeof(DecklinkOutput))]
        [XmlArrayItem(typeof(NdiOutput))]
        [XmlArrayItem(typeof(FFOutput))]
        public OutputBase[] Outputs { get; set; }

        protected override void SetIsModified(bool value)
        {
            base.SetIsModified(value);
            if (value)
                return;
            foreach (var output in Outputs)
                output.IsModified = false;
        }
    }
}
