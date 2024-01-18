using LibAtem.Common;
using OpenMacroBoard.SDK;
using System;
using System.Threading.Tasks;

namespace StudioTVPlayer.Model
{
    public sealed class ElgatoStreamDeckPlayerBinding : PlayerBindingBase
    {
        private readonly int _key;
        public ElgatoStreamDeckPlayerBinding(Configuration.ElgatoStreamDeckPlayerBinding elgatoStreamDeckPlayerBinding)
            : base(elgatoStreamDeckPlayerBinding)
        {
            _key = elgatoStreamDeckPlayerBinding.Key;
        }

        public void KeyPressed(int key)
        {
            if (!(_key == key))
                return;
            ExecuteOnPlayer();
        }

        public int Key => _key;

        public KeyBitmap GetKeyBitmap()
        {
            return KeyBitmap.Create.FromColor(OmbColor.White);
        }
    }
}
