using LibAtem.Common;

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
    }
}
