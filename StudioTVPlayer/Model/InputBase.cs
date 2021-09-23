﻿using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class InputBase
    {
        [XmlAttribute]
        public string VideoFormat { get; set; }

        [XmlIgnore]
        public abstract ImageSource Thumbnail { get; }

        public abstract bool Initialize();
    }
}
