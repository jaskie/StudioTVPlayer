using StudioTVPlayer.Helpers;
using System;
using System.ComponentModel;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class RecordingSchedulerItem : PropertyChangedBase
    {
        private ScheduleRepeatType _repeatType;
        private DayOfWeek[] _repeatDays;
        private bool _isActive;
        private RecordingFilenameCreationRule _filenameCreationRule;
        private string _encoderPreset;
        private TimeSpan _duration = TimeSpan.FromMinutes(10);
        private DateTime _startTime = DateTime.Now.AddHours(1);
        private string _inputId;

        [XmlAttribute]
        public string InputId { get => _inputId; set => Set(ref _inputId, value); }

        public DateTime StartTime { get => _startTime; set => Set(ref _startTime, value); }
        [XmlIgnore]
        public TimeSpan Duration { get => _duration; set => Set(ref _duration, value); }
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
            get => _repeatType; set => Set(ref _repeatType, value);
        }

        /// <summary>
        /// used only for RepeatType.Daily
        /// </summary>
        [XmlElement]
        public DayOfWeek[] RepeatDays { get => _repeatDays; set => Set(ref _repeatDays, value); }

        [XmlAttribute]
        public string Name { get; set; }

        [XmlAttribute]
        public RecordingFilenameCreationRule FilenameCreationRule { get => _filenameCreationRule; set => Set(ref _filenameCreationRule, value); }

        [XmlAttribute]
        public string EncoderPreset { get => _encoderPreset; set => Set(ref _encoderPreset, value); }

        [XmlIgnore]
        public bool IsActive { get => _isActive; set => Set(ref _isActive, value); }

    }
}
