using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class RecordingSchedulerItem
    {
        public DateTime StartTime { get; set; }

        public TimeSpan Duration { get; set; }

        public RepeatType RepeatType { get; set; }

        /// <summary>
        /// used only for RepeatType.Daily
        /// </summary>
        public DayOfWeek[] RepeatDays { get; set; }

        public string Name { get; set; }

        public FilenameCreationRule FilenameCreationRule { get; set; }

    }

    public enum RepeatType
    {
        Single,
        Daily
    }

    public enum FilenameCreationRule
    {
        None,
        DateTimeAtBegin,
        DateTimeAtEnd,
        DateAtBegin,
        DateAtEnd,
        TimeAtBegin,
        TimeAtEnd,
        UseNameAsFormat
    }
}
