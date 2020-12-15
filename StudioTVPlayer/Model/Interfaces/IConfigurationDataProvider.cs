using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using StudioTVPlayer.Model.Args;

namespace StudioTVPlayer.Model.Interfaces
{
    public interface IConfigurationDataProvider
    {
        List<WatcherMeta> GetWatcherMetas();
        List<Channel> GetChannels();
        List<StringWrapper> GetExtensions();
        Configuration LoadConfig();
        void Save();
    }
}
