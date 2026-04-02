using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public class RecordingStore : IDisposable
    {
        private readonly string _folder = Path.Combine(GlobalApplicationData.ApplicationDataDir, "Recordings");
        private readonly XmlSerializer _xmlSerializer = XmlSerializer.FromTypes([typeof(Model.Persistence.Recording)])[0];
        private readonly List<Model.Recording> _runningRecordings = [];
        private readonly List<Model.Recording> _allRecordings;
        private readonly object _runningRecordingsLock = new();
        private readonly object _allRecordingsLock = new();

        public static RecordingStore Current { get; } = new RecordingStore();

        private RecordingStore()
        {
            if (!Directory.Exists(_folder))
            {
                Directory.CreateDirectory(_folder);
            }
            _allRecordings ??= [.. Directory.EnumerateFiles(_folder, "*.xml")
                .Select(fileName =>
                {
                    try
                    {
                    using var reader = new StreamReader(fileName);
                    var deserialized = _xmlSerializer.Deserialize(reader) as Model.Persistence.Recording;
                    return new Model.Recording(deserialized);
                    }
                    catch { } // just ignore non-deserializable files
                    return null;
                 })
                .Where(r => r is not null)
                ];
        }

        public event EventHandler<RecordingEventArgs> RecordingAdded;
        public event EventHandler<RecordingEventArgs> RecordingDeleted;

        public IEnumerable<Model.Recording> RunningRecordings
        {
            get
            {
                lock (_runningRecordingsLock)
                    return [.. _runningRecordings];
            }
        }

        public void SaveRecording(Model.Recording recording)
        {
            var persistenceRecording = new Model.Persistence.Recording(recording);
            using var stream = new FileStream(GetRecordingInfoFilePath(recording), FileMode.Create, FileAccess.Write);
            _xmlSerializer.Serialize(stream, persistenceRecording);
        }

        public IEnumerable<Model.Recording> Recordings
        {
            get
            {
                lock (_allRecordingsLock)
                    return [.. _allRecordings];
            }
        }

        public void AddRecording(Model.Recording recording)
        {
            Debug.Assert(recording.State is Model.RecordingState.Running);
            lock (_runningRecordingsLock)
            {
                _runningRecordings.Add(recording);
            }
            recording.PropertyChanged += Recording_PropertyChanged;
            lock (_allRecordingsLock)
            {
                DeleteRecording(_allRecordings.FirstOrDefault(r => r.FullPath == recording.FullPath));
                _allRecordings.Add(recording);
            }
            RecordingAdded?.Invoke(this, new RecordingEventArgs(recording));
        }

        public void DeleteRecording(Model.Recording recording)
        {
            if (recording is null)
                return;
            lock (_allRecordingsLock)
            {
                if (_allRecordings.Remove(recording))
                    RecordingDeleted?.Invoke(this, new RecordingEventArgs(recording));
            }
            var fileName = GetRecordingInfoFilePath(recording);
            if (File.Exists(fileName))
                File.Delete(fileName);
        }

        private string GetRecordingInfoFilePath(Model.Recording recording) => Path.Combine(_folder, $"{recording.Id}.xml");

        private void Recording_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException($"{nameof(Model.Recording)} expected, {sender?.GetType()} got.");
            if (e.PropertyName is nameof(Model.Recording.State) && recording.State is Model.RecordingState.Aborted or Model.RecordingState.Completed or Model.RecordingState.Failed)
            {
                recording.PropertyChanged -= Recording_PropertyChanged;
                lock (_runningRecordingsLock)
                {
                    _runningRecordings.Remove(recording);
                }
            }
        }

        public void Dispose()
        {
            IEnumerable<Model.Recording> recordingsToStop;
            lock (_runningRecordingsLock)
            {
                recordingsToStop = [.. _runningRecordings];
            }
            foreach (var recording in recordingsToStop)
                recording.Stop(Model.RecordingState.Aborted); //will also call Recording_Finished below
        }
    }

    public class RecordingEventArgs(Model.Recording recording) : EventArgs
    {
        public Model.Recording Recording { get; } = recording;
    }
}
