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
        public string ApplicationVersion
        {
            get
            {
                var version = Assembly.GetExecutingAssembly().GetName().Version;
                return $"{version.Major}.{version.Minor}.{version.Build}";
            }
        }
        public string TVPlayRVersion => TVPlayR.VersionInfo.WrapperVersion;
    }
}
