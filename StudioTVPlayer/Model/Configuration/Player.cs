using System;
using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Configuration
{
    public sealed class Player : ConfigurationItemBase
    {
        private string _name;
        private string _videoFormat;
        private TVPlayR.PixelFormat _pixelFormat;
        private bool _livePreview;
        private bool _disablePlayedItems;
        private bool _addItemsWithAutoPlay;
        private string _id;

        [XmlElement]
        public string Id
        {
            get
            {
                if (string.IsNullOrEmpty(_id))
                    _id = Guid.NewGuid().ToString();
                return _id;
            }
            set => _id = value;
        }

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
        public OutputBase[] Outputs { get; set; } = Array.Empty<OutputBase>();

        protected override void SetIsModified(bool value)
        {
            base.SetIsModified(value);
            if (value)
                return;
            foreach (var output in Outputs)
                output.IsModified = false;
        }

        protected override bool GetIsModified()
        {
            return base.GetIsModified() || Outputs.Any(output => output.IsModified);
        }
    }
}
