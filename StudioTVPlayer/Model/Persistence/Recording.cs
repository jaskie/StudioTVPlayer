using System;
using System.Xml;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model.Persistence
{
    public class Recording
    {
        // Parameterless constructor for serialization
        public Recording()
        {
        }

        public Recording(Model.Recording recording)
        {
            this.InputName = recording.Input.Name;
            this.InputId = recording.Input.Id;
            this.Id = recording.Id.ToString();
            this.StartTime = XmlConvert.ToString(recording.StartTime, XmlDateTimeSerializationMode.Local);
            this.Duration = XmlConvert.ToString(recording.Duration);
            this.File = recording.FullPath;
            this.State = recording.State;
        }

        [XmlAttribute]
        public string Id { get; set; }
        public string StartTime { get; set; }
        public string Duration { get; set; }
        public string File { get; set; }
        public string InputName { get; set; }
        public string InputId { get; set; }
        public RecordingState State { get; set; }

        public DateTime GetStartTime() => XmlConvert.ToDateTime(StartTime, XmlDateTimeSerializationMode.Local);
        public TimeSpan GetDuration() => XmlConvert.ToTimeSpan(Duration);
    }
}
