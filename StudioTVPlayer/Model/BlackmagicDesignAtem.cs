using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class BlackmagicDesignAtem
    {
        private readonly Configuration.BlackmagicDesignAtemPlayerController _configuration;
        private readonly UdpClient _udpClient;
        const int BMDControlPort = 9910;

        public BlackmagicDesignAtem(Configuration.BlackmagicDesignAtemPlayerController configuration)
        {
            _configuration = configuration;
            _udpClient = new UdpClient(configuration.DeviceId, BMDControlPort);
        }

        public void Login()
        {
        }

    }
}
