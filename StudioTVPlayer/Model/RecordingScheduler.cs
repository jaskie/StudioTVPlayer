using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    internal class RecordingScheduler
    {

        private List<RecordingSchedulerItem> _recordings = new List<RecordingSchedulerItem>();

        private RecordingScheduler()
        {

        }

        public static RecordingScheduler Current { get; } = new RecordingScheduler();

        public string OutputDirectory { get; set; }

        public IEnumerable<RecordingSchedulerItem> Recordings
        {
            get
            {
                lock (((IList)_recordings).SyncRoot)
                    return [.. _recordings];
            }
        }

        public bool AddRecording(RecordingSchedulerItem recording)
        {
            lock (((IList)_recordings).SyncRoot)
            {
                if (_recordings.Any(r => r == recording))
                    return false;
                _recordings.Add(recording);
                return true;
            }
        }

    }
}
