using OpenMacroBoard.SDK;
using StreamDeckSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public class ElgatoStreamDeckPlayerController : PlayerControllerBase
    {
        private readonly Configuration.ElgatoStreamDeckPlayerController _playerControllerConfiguration;
        private IMacroBoard _streamDeck;

        public ElgatoStreamDeckPlayerController(Configuration.ElgatoStreamDeckPlayerController playerControllerConfiguration)
        {
            //_streamDeck = StreamDeck.OpenDevice(devicePath, false);
            _playerControllerConfiguration = playerControllerConfiguration;
        }
        public override void Dispose()
        {
        }
    }
}
