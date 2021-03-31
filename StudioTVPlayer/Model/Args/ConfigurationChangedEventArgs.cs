using System;

namespace StudioTVPlayer.Model.Args
{
    public class ConfigurationChangedEventArgs: EventArgs
    {
        public Configuration Configuration { get; }
        public ConfigurationChangedEventArgs(Configuration conf)
        {
            Configuration = conf;
        }
    }
}
