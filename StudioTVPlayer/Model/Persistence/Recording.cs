using System;

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
            this.StartTime = recording.StartTime;
            this.Duration = recording.Duration;
            this.FilePath = recording.FullPath;
            this.State = recording.State;
        }

        public DateTime StartTime { get; set; }
        public TimeSpan Duration { get; set; }
        public string FilePath { get; set; }
        public string InputName { get; set; }
        public RecordingState State { get; set; }
    }
}
