using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    class RecordingScheduled : RecordingBase
    {
        public RecordingScheduled(InputBase input) : base(input) { }

        public DateTime StartTime { get; set; }

        public TimeSpan Duration { get; set; }

        public RepeatType RepeatType { get; set; }

        /// <summary>
        /// used only for RepeatType.Daily
        /// </summary>
        public DayOfWeek[] RepeatDays { get; set; }

        public string Name { get; set; }

    }
}
