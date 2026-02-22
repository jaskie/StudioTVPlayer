using LibAtem.Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class RecordingScheduler : Helpers.IPersistable
    {
        private CancellationTokenSource _cancellationTokenSource;

        private const string FileName = "ScheduledRecordings.xml";

        private bool _isRunnig;

        private RecordingScheduler() { }

        public void Initialize()
        {
            if (_isRunnig)
                return;
            Task.Factory.StartNew(StartRecordingSchedulerLoop, TaskCreationOptions.LongRunning);
        }

        public void Shutdown()
        {
            _isRunnig = false;
            _cancellationTokenSource?.Cancel();
        }

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
            var fullPath = Path.Combine(Providers.GlobalApplicationData.ApplicationDataDir, FileName);
            try
            {
                return Helpers.DataStore.Load<RecordingScheduler>(fullPath) ?? new RecordingScheduler();
            }
            catch
            {
                return new RecordingScheduler();
            }
        }

        /// <summary>
        /// Saves the recordingSchedulerItem list to file and notifies the scheduler about posisible recordings change
        /// </summary>
        public void Save()
        {
            _cancellationTokenSource?.Cancel();
            var fullPath = Path.Combine(Providers.GlobalApplicationData.ApplicationDataDir, FileName);
            lock (((IList)_recordings).SyncRoot)
                Helpers.DataStore.Save(this, fullPath);
        }

        #endregion persistence

        private (RecordingSchedulerItem RecordingSchedulerItem, DateTime StartTime) FindNextRecording()
        {
            lock (((IList)_recordings).SyncRoot)
            {
                RecordingSchedulerItem found = null;
                DateTime foundStartTime = DateTime.MaxValue;
                foreach (var recording in _recordings)
                {
                    if (recording.IsStarted)
                        continue;
                    var currentStartTime = recording.FindNextStartTime();
                    if (currentStartTime < foundStartTime)
                    {
                        found = recording;
                        foundStartTime = currentStartTime;
                    }
                }
                return (found, foundStartTime);
            }
        }

        private async Task StartRecordingSchedulerLoop()
        {
            while (_isRunnig)
            {
                using (_cancellationTokenSource = new CancellationTokenSource())
                {
                    var nextRecording = FindNextRecording();
                    if (nextRecording.RecordingSchedulerItem != null)
                    {
                        await Helpers.WaitUntilHelper.WaitUntilAsync(nextRecording.StartTime, _cancellationTokenSource.Token);
                        if (!_cancellationTokenSource.IsCancellationRequested)
                        {
                            nextRecording.RecordingSchedulerItem.IsStarted = true; // we have to set the flag in the loop thread, otherwise it will be found many times
                            _ = Task.Run(async () => await StartRecording(nextRecording.RecordingSchedulerItem, nextRecording.StartTime + nextRecording.RecordingSchedulerItem.Duration));
                        }
                    }
                    else // recordingSchedulerItem not found - we wait indefinitely for the recordingSchedulerItem collection change
                        await Task.Delay(-1, _cancellationTokenSource.Token);
                }
            }
        }

        private async Task StartRecording(RecordingSchedulerItem recordingSchedulerItem, DateTime endRecordingTime)
        {
            var input = Providers.InputList.Current.Find(recordingSchedulerItem.InputId);
            var encoderPreset = EncoderPresets.Instance.Presets.FirstOrDefault(preset => preset.InputFormats is null); // TODO: preset selection
            var filename = Path.Combine(OutputDirectory, $"{recordingSchedulerItem.Name}.{encoderPreset.FilenameExtension}");
            var recording = new Recording(input, encoderPreset, filename);
            Providers.GlobalApplicationData.Current.AddRecording(recording);
            await Helpers.WaitUntilHelper.WaitUntilAsync(endRecordingTime, CancellationToken.None);
            if (recordingSchedulerItem.RepeatType != ScheduleRepeatType.Single)
            {
                recordingSchedulerItem.IsStarted = false;
                _cancellationTokenSource?.Cancel(); // inform the main loop that it's possible to schedule the recordingSchedulerItem again
            }
            // TODO: else remove the recording from GlobalApplicationData
        }
    }
}
