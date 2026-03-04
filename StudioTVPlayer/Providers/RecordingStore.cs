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
        private readonly XmlSerializer _xmlSerializer = new(typeof(Model.Persistence.Recording));
        private readonly List<Model.Recording> _runningRecordings = [];



        public static RecordingStore Current { get; } = new RecordingStore();

        private RecordingStore()
        {
            if (!Directory.Exists(_folder))
            {
                Directory.CreateDirectory(_folder);
            }
        }

        public IReadOnlyList<Model.Recording> RunningRecordings => _runningRecordings;


        public void SaveRecording(Model.Recording recording)
        {
            var persistenceRecording = new Model.Persistence.Recording(recording);
            using (var stream = new FileStream(GetFilePath(recording), FileMode.Create, FileAccess.Write))
            {
                _xmlSerializer.Serialize(stream, persistenceRecording);
            }
        }

        public IEnumerable<Model.Recording> LoadRecordings()
        {
            Directory.EnumerateFiles(_folder, "*.xml").Select(fileName =>
            {
                using var reader = new StreamReader(fileName);
                var deserialized = _xmlSerializer.Deserialize(reader) as Model.Persistence.Recording;
                var runningRecording = _runningRecordings.FirstOrDefault(r => r.Id.ToString() == deserialized.Id);
                return runningRecording ?? new Model.Recording(deserialized);
            });
            return [];
        }

        public void AddRunningRecording(Model.Recording recording)
        {
            Debug.Assert(recording.State is Model.RecordingState.Running);
            _runningRecordings.Add(recording);
            recording.PropertyChanged += Recording_PropertyChanged;
        }

        internal void DeleteRecording(Model.Recording recording)
        {
            var fileName = GetFilePath(recording);
            if (File.Exists(fileName))
                File.Delete(fileName);
        }

        private string GetFilePath(Model.Recording recording) => Path.Combine(_folder, $"{recording.Id}.xml");
        
        private void Recording_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var recording = sender as Model.Recording ?? throw new ArgumentException(nameof(sender));
            if (e.PropertyName is nameof(Model.Recording.State) && recording.State is Model.RecordingState.Aborted or Model.RecordingState.Completed or Model.RecordingState.Failed)
            {
                recording.PropertyChanged -= Recording_PropertyChanged;
                _runningRecordings.Remove(recording);
            }
        }

        public void Dispose()
        {
            foreach (var recording in _runningRecordings.ToList())
                recording.Dispose(); //will also call Recording_Finished below

        }
    }
}
