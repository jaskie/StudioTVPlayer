using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model.Args
{
    public class ConfigurationChangedEventArgs
    {
        public Configuration Configuration { get; }
        public ConfigurationChangedEventArgs(Configuration conf)
        {
            Configuration = conf;
        }
    }
}
