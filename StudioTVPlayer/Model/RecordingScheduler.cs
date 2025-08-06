using StudioTVPlayer.Providers;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class RecordingScheduler: Helpers.IPersistable
    {

        private const string FileName = "ScheduledRecordings.xml";
        private RecordingScheduler() { }


        [XmlArray(nameof(Recordings))]
        public List<RecordingSchedulerItem> _recordings = [];

        public static RecordingScheduler Current { get; } = Load();

        public string OutputDirectory { get; set; }

        [XmlIgnore]
        public IEnumerable<RecordingSchedulerItem> Recordings
        {
            get
            {
                lock (((IList)_recordings).SyncRoot)
                    return [.. _recordings];
            }
        }

        public void AddRecording(RecordingSchedulerItem recording)
        {
            lock (((IList)_recordings).SyncRoot)
            {
                if (_recordings.Contains(recording))
                    return;
                _recordings.Add(recording);
            }
        }

        public bool RemoveRecording(RecordingSchedulerItem recording)
        {
            lock (((IList)_recordings).SyncRoot)
            {
                return _recordings.Remove(recording);
            }
        }


        #region persistence
        private static RecordingScheduler Load()
        {
            var fullPath = Path.Combine(GlobalApplicationData.ApplicationDataDir, FileName);
            try
            {
                return Helpers.DataStore.Load<RecordingScheduler>(fullPath) ?? new RecordingScheduler();
            }
            catch
            {
                return new RecordingScheduler();
            }
        }

        public void Save()
        {
            var fullPath = Path.Combine(GlobalApplicationData.ApplicationDataDir, FileName);
            Helpers.DataStore.Save(this, fullPath);
        }

        #endregion persistence
    }
}
