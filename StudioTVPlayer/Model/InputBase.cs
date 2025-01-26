using System;
using System.Windows.Media;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public abstract class InputBase : IDisposable
    {
        private string _id;

        [XmlAttribute]
        public string Id
        {
            get
            {
                if (string.IsNullOrEmpty(_id))
                {
                    _id = Guid.NewGuid().ToString();
                }
                return _id;
            }
            set => _id = value;
        }

        [XmlAttribute]
        public string VideoFormat { get; set; }

        [XmlIgnore]
        internal abstract TVPlayR.InputBase TVPlayRInput { get; }

        [XmlIgnore]
        public abstract ImageSource Thumbnail { get; }

        [XmlIgnore]
        public abstract bool IsRunning { get; }

        internal abstract TVPlayR.VideoFormat CurrentFormat();

        public abstract void Dispose();

        public abstract bool Initialize();

        public abstract void Uninitialize();

        internal void AddOutputSink(TVPlayR.OutputSink output)
        {
            TVPlayRInput.AddOutputSink(output);
        }

        internal void RemoveOutputSink(TVPlayR.OutputSink output)
        {
            TVPlayRInput.RemoveOutputSink(output);
        }
    }
}
