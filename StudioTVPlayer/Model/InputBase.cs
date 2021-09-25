using System;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class InputBase: IDisposable
    {
        [XmlAttribute]
        public string VideoFormat { get; set; }

        [XmlIgnore]
        public abstract ImageSource Thumbnail { get; }

        public abstract bool IsRunning { get; }

        public abstract void Dispose();
        
        public abstract bool Initialize();
        
        public abstract void Uninitialize();


    }
}
