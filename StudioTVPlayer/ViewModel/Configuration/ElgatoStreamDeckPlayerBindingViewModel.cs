using System.Diagnostics;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class ElgatoStreamDeckPlayerBindingViewModel : PlayerControllerBindingViewModelBase
    {
        private int _key = 1;

        public ElgatoStreamDeckPlayerBindingViewModel(Model.Configuration.ElgatoStreamDeckPlayerBinding bindingConfiguration = null) : base(bindingConfiguration ?? new Model.Configuration.ElgatoStreamDeckPlayerBinding())
        {
            if (bindingConfiguration != null)
            {
                _key = bindingConfiguration.Key;
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
            base.Apply();
        }

        public int Key { get => _key; set => Set(ref _key, value); }

        internal void KeyReceived(int key)
        {
            if (!IsListening)
                return;
            Key = key;
            IsListening = false;
        }
    }
}
