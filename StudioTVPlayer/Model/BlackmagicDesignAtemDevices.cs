using LibAtem.Net;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace StudioTVPlayer.Model
{
    public static class BlackmagicDesignAtemDevices
    {
        private static ConcurrentDictionary<string, AtemClient> _devices = new ConcurrentDictionary<string, AtemClient>();

        public static AtemClient GetDevice(string address)
        {
            return _devices.GetOrAdd(address, c => new AtemClient(c));
        }

        public static bool CloseDevice(string address)
        {
            if (_devices.TryRemove(address, out var device))
            {
                device.Dispose();
                return true;
            }
            return false;
        }
    }
}
