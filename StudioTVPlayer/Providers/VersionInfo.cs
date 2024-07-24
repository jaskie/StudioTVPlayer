using System.Reflection;

namespace StudioTVPlayer.Providers
{
    public class VersionInfo
    {
        private VersionInfo() { }

        public static VersionInfo Current { get; } = new VersionInfo();

        public string Application
        {
            get
            {
                var version = Assembly.GetExecutingAssembly().GetName().Version;
                return $"{version.Major}.{version.Minor}.{version.Build}";
            }
        }
        public string Wrapper { get; } = TVPlayR.VersionInfo.Wrapper;

        public string FFmpegAVFormat { get; } = TVPlayR.VersionInfo.FFmpegAVFormat;

        public string FFmpegAVCodec { get; } = TVPlayR.VersionInfo.FFmpegAVCodec;
        
        public string FFmpegAVFilter { get; } = TVPlayR.VersionInfo.FFmpegAVFilter;

        public string Ndi { get; } = TVPlayR.VersionInfo.Ndi ?? "not found";

        public string Decklink { get; } = TVPlayR.VersionInfo.Decklink ?? "not found";
    }
}
