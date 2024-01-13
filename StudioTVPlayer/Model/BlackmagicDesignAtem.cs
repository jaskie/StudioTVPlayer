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
    public sealed class BlackmagicDesignAtem : IDisposable
    {
        private static ConcurrentDictionary<string, BlackmagicDesignAtem> _devices = new ConcurrentDictionary<string, BlackmagicDesignAtem>();
        public static IDictionary<string, BlackmagicDesignAtem> Devices => _devices;
        public static BlackmagicDesignAtem GetDevice(string address)
        {
            return _devices.GetOrAdd(address, c => new BlackmagicDesignAtem(c));
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


        private readonly UdpClient _client;
        private readonly IPEndPoint _remoteEndpoint;
        private bool _disposed;

        public event EventHandler Connected;

        private BlackmagicDesignAtem(string address)
        {
            var addressParts = address.Split(new char[] { ':' }, 2);
            int port = 9910;
            if (addressParts.Length == 2)
                int.TryParse(addressParts[1], out port);
            _remoteEndpoint = new IPEndPoint(IPAddress.Parse(addressParts[0]), port);
            _client = new UdpClient(new IPEndPoint(IPAddress.Any, 0));
            StartReceiving();
            Handshake();
        }

        private void Handshake()
        {
        }

        private void StartReceiving()
        {
            var thread = new Thread(() =>
            {
                while (!_disposed)
                {
                    try
                    {
                        var remoteEndpoint = _remoteEndpoint;
                        var data = _client.Receive(ref remoteEndpoint);
                        OnReceived(data);
                    }
                    catch (SocketException) { } //ignore it
                }
                Debug.WriteLine($"Receiving thread for {_remoteEndpoint} finished.");
            });
            thread.Name = $"Recieiving thread for {_remoteEndpoint}";
            thread.IsBackground = true;
            thread.Start();
        }

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            _client.Dispose();
        }

        private void OnReceived(byte[] data)
        {

        }
    }

    public enum BlackmagicAtemMe
    {
        Me1,
        Me2,
        Me3,
        Me4,
    }

    public enum BlackmagicAtemVideoSource
    {
        Black,
        Input1,
        Input2,
        Input3,
        Input4,
        Input5,
        Input6,
        Input7,
        Input8,
        Input9,
        Input10,
        Input11,
        Input12,
        Input13,
        Input14,
        Input15,
        Input16,
        Input17,
        Input18,
        Input19,
        Input20,
        Input21,
        Input22,
        Input23,
        Input24,
        Input25,
        Input26,
        Input27,
        Input28,
        Input29,
        Input30,
        Input31,
        Input32,
        Input33,
        Input34,
        Input35,
        Input36,
        Input37,
        Input38,
        Input39,
        Input40,
        ColorBars = 1000,
        Color1 = 2001,
        Color2,
        MediaPlayer1 = 3010,
        MediaPlayer1Key,
        MediaPlayer2 = 3020,
        MediaPlayer2Key,
        MediaPlayer3 = 3030,
        MediaPlayer3Key,
        MediaPlayer4 = 3040,
        MediaPlayer4Key,
        Key1Mask = 4010,
        Key2Mask = 4020,
        Key3Mask = 4030,
        Key4Mask = 4040,
        Key5Mask = 4050,
        Key6Mask = 4060,
        Key7Mask = 4070,
        Key8Mask = 4080,
        Key9Mask = 4090,
        Key10Mask = 4100,
        Key11Mask = 4110,
        Key12Mask = 4120,
        Key13Mask = 4130,
        Key14Mask = 4140,
        Key15Mask = 4150,
        Key16Mask = 4160,
        DSK1Mask = 5010,
        DSK2Mask = 5020,
        DSK3Mask = 5030,
        DSK4Mask = 5040,
        SuperSource = 6000,
        SuperSource2 = 6001,
        CleanFeed1 = 7001,
        CleanFeed2,
        CleanFeed3,
        CleanFeed4,
        Auxilary1 = 8001,
        Auxilary2,
        Auxilary3,
        Auxilary4,
        Auxilary5,
        Auxilary6,
        Auxilary7,
        Auxilary8,
        Auxilary9,
        Auxilary10,
        Auxilary11,
        Auxilary12,
        Auxilary13,
        Auxilary14,
        Auxilary15,
        Auxilary16,
        Auxilary17,
        Auxilary18,
        Auxilary19,
        Auxilary20,
        Auxilary21,
        Auxilary22,
        Auxilary23,
        Auxilary24,
        ME1Prog = 10010,
        ME1Prev,
        ME2Prog = 10020,
        ME2Prev,
        ME3Prog = 10030,
        ME3Prev,
        ME4Prog = 10040,
        ME4Prev,
        Input1Direct = 11001,
    }
}
