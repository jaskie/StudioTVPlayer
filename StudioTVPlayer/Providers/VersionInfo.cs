using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

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
        public string Wrapper => TVPlayR.VersionInfo.Wrapper;

        public string FFmpegAVFormat => TVPlayR.VersionInfo.FFmpegAVFormat;
    }
}
