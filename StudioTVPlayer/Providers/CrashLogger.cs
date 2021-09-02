using System;
using System.IO;

namespace StudioTVPlayer.Providers
{
    public static class CrashLogger
    {
        private const string PathName = "StudioTVPlayer";
        public static readonly string ApplicationDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), PathName);
        public static void SaveDump(string text)
        {
            var fileName = Path.Combine(ApplicationDataDir, $"{DateTime.Now:yyyyMMdd HHmmss}.log");
            using (var stream = File.CreateText(fileName))
            {
                stream.Write(text);
                stream.Close();
            }
        }
    }
}
