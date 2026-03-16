using System.IO;
using System.Xml.Serialization;

namespace StudioTVPlayer.Helpers
{

    public interface IPersistable { }

    public static class DataStore
    {
        public static void Save<T>(this T data, string fileName) where T : IPersistable
        {
            var dirName = Path.GetDirectoryName(fileName);
            if (!Directory.Exists(dirName))
                Directory.CreateDirectory(dirName);
            var serializer = XmlSerializer.FromTypes([data.GetType()])[0];
            using (var writer = new StreamWriter(fileName))
                serializer.Serialize(writer, data);
        }

        public static T Load<T>(string fileName) where T : IPersistable
        {
            if (!File.Exists(fileName))
                return default;
            using var reader = new StreamReader(fileName);
            var serializer = XmlSerializer.FromTypes([typeof(T)])[0];
            return (T)serializer.Deserialize(reader);
        }
    }
}
