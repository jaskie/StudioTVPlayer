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
            var serializer = new XmlSerializer(data.GetType());
            using (var writer = new StreamWriter(fileName))
                serializer.Serialize(writer, data);
        }

        public static T Load<T>(string fileName) where T : IPersistable
        {
            if (!File.Exists(fileName))
                return default;
            using (var reader = new StreamReader(fileName))
            {
                var serializer = new XmlSerializer(typeof(T));
                return (T)serializer.Deserialize(reader);
            }
        }
    }
}
