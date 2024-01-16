using LibAtem.Net;
using System.Collections.Generic;

namespace StudioTVPlayer.Model
{
    public static class BlackmagicDesignAtemDevices
    {
        private class AtemClientReference
        {
            public int UseCount;
            public AtemClient AtemClient;
        }

        private static object _lock = new object();
        private static Dictionary<string, AtemClientReference> _devices = new Dictionary<string, AtemClientReference>();

        public static AtemClient GetDevice(string address)
        {
            lock (_lock)
            {
                if (!_devices.TryGetValue(address, out var reference))
                {
                    reference = new AtemClientReference { AtemClient = new AtemClient(address) };
                    _devices[address] = reference;
                }
                reference.UseCount++;
                return reference.AtemClient;
            }
        }

        public static bool ReleaseDevice(string address)
        {
            lock (_lock)
            {
                if (!_devices.TryGetValue(address, out var reference))
                    return false;
                reference.UseCount--;
                if (reference.UseCount == 0)
                {
                    _devices.Remove(address);
                    reference.AtemClient.Dispose();
                }
                return true;
            }
        }
    }
}
