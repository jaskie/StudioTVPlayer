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
        private IMacroBoard _streamDeck;

        public ElgatoStreamDeckPlayerController(string devicePath)
        {
            _streamDeck = StreamDeck.OpenDevice(devicePath, false);
        }
        public override void Dispose()
        {
        }
    }
}
