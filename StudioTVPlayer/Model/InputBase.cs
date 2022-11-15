using System;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class InputBase : IDisposable
    {
        [XmlAttribute]
        public string VideoFormat { get; set; }

        [XmlIgnore]
        public abstract TVPlayR.InputBase Input { get; }

        [XmlIgnore]
        public abstract ImageSource Thumbnail { get; }

        [XmlIgnore]
        public abstract bool IsRunning { get; }

        public abstract void Dispose();
        
        public abstract bool Initialize();
        
        public abstract void Uninitialize();

        public void AddOutputSink(TVPlayR.OutputSink output)
        {
            Input.AddOutputSink(output);
        }

        public void RemoveOutputSink(TVPlayR.FFOutput output)
        {
            Input.RemoveOutputSink(output);
        }
    }
}
