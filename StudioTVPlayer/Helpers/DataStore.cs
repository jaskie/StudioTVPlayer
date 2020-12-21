using System;
using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Helpers
{
    public static class DataStore
    {
        private const string PathName = "StudioTVPlayer";
        private static readonly string AppData = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
        private static readonly string ApplicationDataDir = Path.Combine(AppData, PathName);
        
        public static void Save<T>(this T data, string fileName)
        {                   
            if (!Directory.Exists(ApplicationDataDir))
                Directory.CreateDirectory(ApplicationDataDir);
            var fullPath = $"{Path.Combine(ApplicationDataDir, fileName)}";
            var serializer = new XmlSerializer(data.GetType());
            using (var writer = new StreamWriter(fullPath))
                serializer.Serialize(writer, data);
        }

        public static T Load<T>(string fileName)
        {
            var fullPath = $"{Path.Combine(ApplicationDataDir, fileName)}";
            if (!File.Exists(fullPath))
                return default(T);

            using (var reader = new StreamReader(fullPath))
            {
                var serializer = new XmlSerializer(typeof(T));
                return (T)serializer.Deserialize(reader);
            }
        }
    }
}
