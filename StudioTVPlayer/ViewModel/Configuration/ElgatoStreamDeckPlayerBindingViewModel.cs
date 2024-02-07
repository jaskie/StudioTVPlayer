using System;
using System.Diagnostics;
using System.Globalization;
using System.Windows.Media;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ElgatoStreamDeckPlayerBindingViewModel : PlayerControllerBindingViewModelBase
    {
        private int _key = 1;
        private Color _buttonBackgroundColor = Colors.Red;

        public ElgatoStreamDeckPlayerBindingViewModel(Model.Configuration.ElgatoStreamDeckPlayerBinding bindingConfiguration = null) : base(bindingConfiguration ?? new Model.Configuration.ElgatoStreamDeckPlayerBinding())
        {
            if (bindingConfiguration != null)
            {
                _key = bindingConfiguration.Key;
                if (int.TryParse(bindingConfiguration.ButtonBackgroundColor, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out var intColor))
                {
                    var colorsArray = BitConverter.GetBytes(intColor);
                    _buttonBackgroundColor = Color.FromRgb(colorsArray[2], colorsArray[1], colorsArray[0]);
                }
            }
        }

        public override bool IsValid()
        {
            return true;
        }

        public override void Apply()
        {
            var bindingConfiguration = BindingConfiguration as Model.Configuration.ElgatoStreamDeckPlayerBinding;
            Debug.Assert(bindingConfiguration != null);
            bindingConfiguration.Key = _key;
            bindingConfiguration.ButtonBackgroundColor = string.Format("{0:X2}{1:X2}{2:X2}", _buttonBackgroundColor.R, _buttonBackgroundColor.G, _buttonBackgroundColor.B);
            base.Apply();
        }

        public int Key { get => _key; set => Set(ref _key, value); }

        public Color ButtonBackgroundColor { get => _buttonBackgroundColor; set => Set(ref _buttonBackgroundColor, value); }

        internal void KeyReceived(int key)
        {
            if (!IsListening)
                return;
            Key = key;
            IsListening = false;
        }
    }
}
