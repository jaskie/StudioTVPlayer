using LibAtem.Common;
using OpenMacroBoard.SDK;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace StudioTVPlayer.Model
{
    public sealed class ElgatoStreamDeckPlayerBinding : PlayerBindingBase
    {
        private readonly int _key;
        private bool _isEnabled;
        private string _icon;

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


        /// <summary>
        /// Returns KeyBitmap for a key. Returns null if no change is required
        /// </summary>
        public KeyBitmap GetKeyBitmap(Font font, int keySize, RundownPlayer player = null)
        {
            var bitmapImage = new Bitmap(keySize, keySize, PixelFormat.Format24bppRgb);
            var icon = MethodToIcon(PlayerMethod, player?.PlayerState ?? PlayerState.Disabled);
            bool isEnabled = CanEnable(player);
            if (isEnabled == _isEnabled && icon == _icon)
                return null;
            _isEnabled = isEnabled;
            _icon = icon;
            using (var g = Graphics.FromImage(bitmapImage))
            {
                g.FillRectangle(isEnabled ? Brushes.Red : Brushes.DarkGray, 0, 0, keySize, keySize);
                var size = g.MeasureString(icon, font);
                g.DrawString(icon, font, isEnabled ? Brushes.White : Brushes.LightGray, (keySize - size.Width) / 2, (keySize - size.Height) / 2);
                return KeyBitmap.Create.FromBgr24Array(keySize, keySize, BitmapToByteArray(bitmapImage));
            }
        }

        private static string MethodToIcon(PlayerMethodKind playerMethodKind, PlayerState playerState)
        {
            switch (playerMethodKind)
            {
                case PlayerMethodKind.Cue:
                    return "\xE892";
                case PlayerMethodKind.LoadNext:
                    return "\xF8AD";
                case PlayerMethodKind.Pause:
                case PlayerMethodKind.Toggle when playerState is PlayerState.Playing:
                    return "\xE769";
                case PlayerMethodKind.Play:
                case PlayerMethodKind.Toggle:
                    return "\xE768";
                case PlayerMethodKind.Clear:
                    return "\xE71A";
                default:
                    throw new ArgumentException(nameof(playerMethodKind));
            }
        }

        private static byte[] BitmapToByteArray(Bitmap bitmap)
        {
            BitmapData bmpdata = null;
            try
            {
                bmpdata = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, bitmap.PixelFormat);
                var bytedata = new byte[bmpdata.Stride * bitmap.Height];
                var ptr = bmpdata.Scan0;
                Marshal.Copy(ptr, bytedata, 0, bytedata.Length);
                return bytedata;
            }
            finally
            {
                if (bmpdata != null)
                    bitmap.UnlockBits(bmpdata);
            }
        }

        private bool CanEnable(RundownPlayer player)
        {
            if (player is null)
                return false;
            var playerState = player.PlayerState;
            switch (PlayerMethod)
            {
                case PlayerMethodKind.Cue:
                    return player.CanCue();
                case PlayerMethodKind.LoadNext:
                    return player.CanLoadNextItem();
                case PlayerMethodKind.Play:
                    return playerState == PlayerState.Cue || playerState == PlayerState.Paused;
                case PlayerMethodKind.Pause:
                    return playerState == PlayerState.Playing;
                case PlayerMethodKind.Toggle:
                    return playerState != PlayerState.Finished && (playerState == PlayerState.Cue || playerState == PlayerState.Paused || playerState == PlayerState.Playing);
                case PlayerMethodKind.Clear:
                    return player.IsLoaded();
                default:
                    return false;
            }
        }
    }
}
