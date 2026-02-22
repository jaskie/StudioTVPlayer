using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Providers
{
    public static class RecordingPersister
    {
        private static readonly string _folder = Path.Combine(GlobalApplicationData.ApplicationDataDir, "Recordings");
        private static readonly XmlSerializer _xmlSerializer = new XmlSerializer(typeof(Model.Persistence.Recording));

        static RecordingPersister()
        {
            if (!Directory.Exists(_folder))
            {
                Directory.CreateDirectory(_folder);
            }
        }

        public static void SaveRecording(Model.Recording recording)
        {
            var persistenceRecording = new Model.Persistence.Recording(recording);
            var filePath = Path.Combine(_folder, $"{recording.Id}.xml");
            using (var stream = new FileStream(filePath, FileMode.Create, FileAccess.Write))
            {
                _xmlSerializer.Serialize(stream, persistenceRecording);
            }
        }

        public static IEnumerable<Model.Recording> LoadRecordings()
        {
            return [];
        }

        internal static void DeleteRecording(Model.Recording recording)
        {
            throw new NotImplementedException();
        }
    }
}
