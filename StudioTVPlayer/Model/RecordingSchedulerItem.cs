using System;
using System.ComponentModel;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class RecordingSchedulerItem
    {
        private ScheduleRepeatType _repeatType;
        private DayOfWeek[] _repeatDays;

        [XmlAttribute]
        public string InputId { get; set; }

        public DateTime StartTime { get; set; } = DateTime.Now.AddHours(1);

        [XmlIgnore]
        public TimeSpan Duration { get; set; } = TimeSpan.FromMinutes(10);

        [XmlElement(nameof(Duration))]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public long DurationTicks
        {
            get => Duration.Ticks;
            set => Duration = new TimeSpan(value);
        }

        [XmlAttribute]
        public ScheduleRepeatType RepeatType
        {
            get => _repeatType; set => _repeatType = value;
        }

        /// <summary>
        /// used only for RepeatType.Daily
        /// </summary>
        [XmlElement]
        public DayOfWeek[] RepeatDays { get => _repeatDays; set => _repeatDays = value; }

        [XmlAttribute]
        public string Name { get; set; }

        [XmlAttribute]
        public FilenameCreationRule FilenameCreationRule { get; set; }

        [XmlAttribute]
        public string EncoderPreset { get; set; }

        [XmlIgnore]
        public bool IsStarted { get; set; }

        internal DateTime FindNextStartTime()
        {
            // TODO: update for daily repeats
            return StartTime;
        }
    }
}
