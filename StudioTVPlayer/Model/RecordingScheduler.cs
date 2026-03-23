using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public class RecordingScheduler : Helpers.IPersistable
    {
        private CancellationTokenSource _mainLoopWaitCancellationTokenSource;

        private const string FileName = "ScheduledRecordings.xml";

        private bool _isRunnig;

        private RecordingScheduler() { }

        public void Initialize()
        {
            if (_isRunnig)
                return;
            Task.Factory.StartNew(RecordingSchedulerLoop, TaskCreationOptions.LongRunning);
        }

        public void Shutdown()
        {
            _isRunnig = false;
            NotifyMainLoop();
        }

        [XmlArray(nameof(Recordings))]
        public List<RecordingSchedulerItem> _recordings = [];

        public static RecordingScheduler Current { get; } = Load();

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
            NotifyMainLoop();
            var fullPath = Path.Combine(Providers.GlobalApplicationData.ApplicationDataDir, FileName);
            lock (((IList)_recordings).SyncRoot)
                Helpers.DataStore.Save(this, fullPath);
        }

        #endregion persistence

        private (RecordingSchedulerItem RecordingSchedulerItem, DateTime StartTime) FindNextRecordingSchedulerItem()
        {
            lock (((IList)_recordings).SyncRoot)
            {
                RecordingSchedulerItem foundRecordingItem = null;
                DateTime foundStartTime = DateTime.MaxValue;
                var now = DateTime.Now;
                foreach (var recording in _recordings)
                {
                    if (recording.IsActive)
                        continue;
                    var currentStartTime = FindNextStartTime(recording, ref now);
                    if (currentStartTime != default && currentStartTime < foundStartTime)
                    {
                        foundRecordingItem = recording;
                        foundStartTime = currentStartTime;
                    }
                }
                return (foundRecordingItem, foundStartTime);
            }
        }

        private async Task RecordingSchedulerLoop()
        {
            _isRunnig = true;
            while (_isRunnig)
            {
                using (_mainLoopWaitCancellationTokenSource = new CancellationTokenSource())
                {
                    var nextRecordingInfo = FindNextRecordingSchedulerItem();
                    if (nextRecordingInfo.RecordingSchedulerItem != null)
                    {
                        await Helpers.WaitUntilHelper.WaitUntilAsync(nextRecordingInfo.StartTime, _mainLoopWaitCancellationTokenSource.Token);
                        if (!_mainLoopWaitCancellationTokenSource.IsCancellationRequested)
                        {
                            nextRecordingInfo.RecordingSchedulerItem.IsActive = true; // we have to set the flag in the loop thread, otherwise it will be found many times
                            _ = Task.Run(async () => await StartRecording(nextRecordingInfo.RecordingSchedulerItem));
                        }
                    }
                    else // recordingSchedulerItem not found - we wait indefinitely for the recordingSchedulerItem collection change
                        await Task.Delay(-1, _mainLoopWaitCancellationTokenSource.Token);
                }
            }
        }

        public async Task StartRecording(RecordingSchedulerItem recordingSchedulerItem)
        {
            var input = Providers.InputList.Current.Find(recordingSchedulerItem.InputId);
            var encoderPreset = EncoderPresets.Instance.Presets.FirstOrDefault(preset => preset.PresetName == recordingSchedulerItem.EncoderPreset);
            var directory = recordingSchedulerItem.Folder;
            if (!Directory.Exists(directory))
                Directory.CreateDirectory(directory);
            var filename = Path.Combine(directory, $"{recordingSchedulerItem.Name}.{encoderPreset.FilenameExtension}");
            var recording = new Recording(input, encoderPreset, filename);
            recording.Start();
            // we will always wait for the duration to pass, esp. if the recording jsut failed - we don't want to restart it automatically
            await Helpers.WaitUntilHelper.WaitForAsync(recordingSchedulerItem.Duration, CancellationToken.None);
            recording.Stop();
            recordingSchedulerItem.IsActive = false;
            if (recordingSchedulerItem.RepeatType != ScheduleRepeatType.Single)
            {
                NotifyMainLoop(); // inform the main loop that it's possible to schedule the recordingSchedulerItem again
            }
        }

        private void NotifyMainLoop()
        {
            var oldToken = Interlocked.Exchange(ref _mainLoopWaitCancellationTokenSource, null);
            oldToken?.Cancel();
        }

        private static DateTime FindNextStartTime(RecordingSchedulerItem item, ref DateTime now)
        {
            TimeSpan startTod = item.StartTime.TimeOfDay;
            TimeSpan duration = item.Duration;

            switch (item.RepeatType)
            {
                case ScheduleRepeatType.Single:
                    DateTime singleStart = item.StartTime;
                    DateTime singleEnd = singleStart + duration;

                    // If it hasn't finished yet, the "start" is the scheduled start
                    if (now < singleEnd)
                        return singleStart;
                    return default;

                case ScheduleRepeatType.Daily:
                    // 1. Check if we are currently in "yesterday's" wrapped window
                    // (Only relevant if duration > time since midnight)
                    DateTime yesterdayStart = now.Date.AddDays(-1).Add(startTod);
                    if (item.RepeatDays.Contains(yesterdayStart.DayOfWeek) &&
                        now < yesterdayStart + duration)
                        return yesterdayStart;

                    // 2. Check "today's" repetition and time window
                    DateTime todayStart = now.Date.Add(startTod);
                    if (item.RepeatDays.Contains(todayStart.DayOfWeek) &&
                        now >= todayStart && now < todayStart + duration)
                        return todayStart;

                    // 3. Otherwise, the next is in the future
                    if (TryFindDaysDelta(todayStart.DayOfWeek, item.RepeatDays, out var daysToWait))
                        return todayStart.AddDays(daysToWait);
                    return default;
            }
            return default;
        }

        private static bool TryFindDaysDelta(DayOfWeek today, DayOfWeek[] repeatDays, out int daysDelta)
        {
            daysDelta = default;
            if (repeatDays == null || repeatDays.Length == 0)
                return false;

            for (int i = 1; i <= 7; i++)
            {
                DayOfWeek candidate = (DayOfWeek)(((int)today + i) % 7);
                if (repeatDays.Contains(candidate))
                {
                    daysDelta = i;
                    return true;
                }
            }
            return false;
        }
    }
}
