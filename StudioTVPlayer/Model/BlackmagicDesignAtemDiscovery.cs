using Makaretu.Dns;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Threading;

namespace StudioTVPlayer.Model
{
    public sealed class BlackmagicDesignAtemDiscovery: IDisposable
    {
        private const string ServiceName = "_blackmagic._tcp.local";

        private readonly ConcurrentBag<BlackmagicDesignAtemDeviceInfo> _knownDevices;
        private readonly MulticastService _mdns;
        private readonly int _considerMissingTime;
        private readonly Timer _updateTimer;
        private bool _disposed;

        public event EventHandler<BlackmagicAtemDeviceEventArgs> DeviceSeen;
        public event EventHandler<BlackmagicAtemDeviceEventArgs> DeviceUpdated;
        public event EventHandler<BlackmagicAtemDeviceEventArgs> DeviceLost;

        public BlackmagicDesignAtemDiscovery(int updatePeriod = 10000)
        {
            _knownDevices = new ConcurrentBag<BlackmagicDesignAtemDeviceInfo>();

            _mdns = new MulticastService();
            _mdns.UseIpv4 = true;
            _mdns.UseIpv6 = false;
            _mdns.NetworkInterfaceDiscovered += (s, e) => _mdns.SendQuery(ServiceName);
            _mdns.AnswerReceived += AnswerReceived;
            _mdns.Start();
            _updateTimer = new Timer(TimerCallback, null, updatePeriod, updatePeriod);
            _considerMissingTime = updatePeriod * 3;
        }

        public IReadOnlyCollection<BlackmagicDesignAtemDeviceInfo> Devices => _knownDevices.ToArray();

        private void AnswerReceived(object _, MessageEventArgs args)
        {
            var answer = args.Message.Answers.OfType<PTRRecord>().FirstOrDefault(a => a.Name == ServiceName);
            if (answer == null)
                return;

            var records = args.Message.AdditionalRecords;

            var srvRec = records.OfType<SRVRecord>().FirstOrDefault(r => r.Type == DnsType.SRV && r.Name == answer.DomainName);
            if (srvRec == null)
            {
                Debug.WriteLine($"Missing SRV record for {answer.DomainName}");
                return;
            }

            var aRec = records.OfType<AddressRecord>().FirstOrDefault(r => r.Type == DnsType.A && r.Name == srvRec.Target);
            if (aRec == null)
            {
                Debug.WriteLine($"Missing SRV record for {answer.DomainName}");
                return;
            }

            var txtRec = records.OfType<TXTRecord>().FirstOrDefault(r => r.Type == DnsType.TXT && r.Name == answer.DomainName);
            if (txtRec?.Strings is List<string> strings)
            {
                string id = null;
                string modelName = null;
                foreach (var s in strings)
                {
                    var splitted = s.Split(new[] { '=' }, 2);
                    if (splitted.Length != 2)
                        continue;
                    switch (splitted[0])
                    {
                        case "name":
                            modelName = splitted[1];
                            break;
                        case "unique id":
                            id = splitted[1];
                            break;
                    }
                }
                if (string.IsNullOrEmpty(id) || string.IsNullOrEmpty(modelName))
                    return;
                string deviceName = string.Join(".", answer.DomainName.Labels);
                if (deviceName.EndsWith(ServiceName))
                    deviceName = deviceName.Substring(0, deviceName.Length - ServiceName.Length - 1);

                var dev = _knownDevices.FirstOrDefault(d => d.DeviceId == id);
                if (dev == null)
                {
                    dev = new BlackmagicDesignAtemDeviceInfo(id, modelName, deviceName, DateTime.Now, aRec.Address, srvRec.Port);
                    _knownDevices.Add(dev);
                    DeviceSeen?.Invoke(this, new BlackmagicAtemDeviceEventArgs(dev));
                }
                else
                if (dev.Update(deviceName, DateTime.Now, aRec.Address, srvRec.Port))
                    DeviceUpdated?.Invoke(this, new BlackmagicAtemDeviceEventArgs(dev));
            }
        }

        private void TimerCallback(object _)
        {
            DateTime now = DateTime.Now;
            foreach (var dev in _knownDevices)
            {
                // Remove if not seen in too long
                if (dev != null && now.Subtract(dev.LastSeen).TotalMilliseconds > _considerMissingTime)
                {
                    if (_knownDevices.TryTake(out var device))
                        DeviceLost?.Invoke(this, new BlackmagicAtemDeviceEventArgs(device));
                }
            }
            _mdns.SendQuery(ServiceName);
        }

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;
            _mdns.AnswerReceived -= AnswerReceived;
            _mdns.Stop();
            _mdns.Dispose();
            _updateTimer.Dispose();
        }
    }

    public class BlackmagicDesignAtemDeviceInfo
    {
        public string DeviceId { get; }
        public string ModelName { get; }
        public string DeviceName { get; private set; }
        public DateTime LastSeen { get; private set; }
        public IPAddress Address { get; private set; }
        public int Port { get; private set; }

        public BlackmagicDesignAtemDeviceInfo(string deviceId, string modelName, string deviceName, DateTime lastSeen, IPAddress address, int port)
        {
            DeviceId = deviceId;
            ModelName = modelName;
            DeviceName = deviceName;
            LastSeen = lastSeen;
            Address = address;
            Port = port;
        }

        internal bool Update(string deviceName, DateTime lastSeen, IPAddress address, int port)
        {
            if (DeviceName == deviceName && LastSeen == lastSeen && Address == address && Port == port)
                return false;
            DeviceName = deviceName;
            LastSeen = lastSeen;
            Address = address;
            Port = port;
            return true;
        }
    }

    public class BlackmagicAtemDeviceEventArgs : EventArgs
    {
        public BlackmagicAtemDeviceEventArgs(BlackmagicDesignAtemDeviceInfo device)
        {
            Device = device;
        }

        public BlackmagicDesignAtemDeviceInfo Device { get; }
    }
}
