using Makaretu.Dns;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
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
        private readonly int _updatePeriod;
        private readonly Timer _updateTimer;
        private bool _disposed;

        public event EventHandler<BlackmagicAtemDeviceEventArgs> DeviceSeen;
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
            _updatePeriod = updatePeriod;
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
            List<string> strings = txtRec?.Strings ?? new List<string>();

            string name = string.Join(".", answer.DomainName.Labels);
            if (name.EndsWith(ServiceName))
                name = name.Substring(0, name.Length - ServiceName.Length - 1);

            var deviceId = srvRec.Target.ToString();
            var dev = _knownDevices.FirstOrDefault(d => d.DeviceId == deviceId);
            if (dev == null)
            {
                dev = new BlackmagicDesignAtemDeviceInfo(deviceId, name, DateTime.Now, aRec.Address, srvRec.Port, strings);
                _knownDevices.Add(dev);
                DeviceSeen?.Invoke(this, new BlackmagicAtemDeviceEventArgs(dev));
            }
            else
                dev.Update(name, DateTime.Now, aRec.Address, srvRec.Port, strings);
        }

        private void TimerCallback(object _)
        {
            DateTime now = DateTime.Now;

            foreach (var dev in _knownDevices)
            {
                // Remove if not seen in too long
                if (dev != null && now.Subtract(dev.LastSeen).TotalMilliseconds > _updatePeriod * 3)
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

    public class BlackmagicDesignAtemDeviceInfo: Helpers.PropertyChangedBase
    {
        private string _name;
        private DateTime _lastSeen;
        private IPAddress _address;
        private int _port;
        private IReadOnlyList<string> _strings;

        public string DeviceId { get; }
        public string Name { get => _name; private set => Set(ref _name, value); }
        public DateTime LastSeen { get => _lastSeen; private set => Set(ref _lastSeen, value); }

        public IPAddress Address { get => _address; private set => Set(ref _address, value); }
        public int Port { get => _port; private set => Set(ref _port, value); }

        public IReadOnlyList<string> Strings { get => _strings; private set => Set(ref _strings, value); }

        public BlackmagicDesignAtemDeviceInfo(string deviceId, string name, DateTime lastSeen, IPAddress address, int port, IReadOnlyList<string> strings)
        {
            DeviceId = deviceId;
            _name = name;
            _lastSeen = lastSeen;
            _address = address;
            _port = port;
            _strings = strings;
        }

        internal void Update(string name, DateTime lastSeen, IPAddress address, int port, IReadOnlyList<string> strings)
        {
            Name = name;
            LastSeen = lastSeen;
            Address = address;
            Port = port;
            Strings = strings;
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
