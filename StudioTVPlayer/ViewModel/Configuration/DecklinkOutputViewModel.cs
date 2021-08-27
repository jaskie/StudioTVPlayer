using StudioTVPlayer.Model;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class DecklinkOutputViewModel : OutputViewModelBase
    {
        private DecklinkOutput _decklink;
        private TVPlayR.DecklinkInfo _selectedDevice;

        public DecklinkOutputViewModel(DecklinkOutput decklink): base(decklink)
        {
            _decklink = decklink;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == decklink.DeviceIndex);
        }

        public static TVPlayR.DecklinkInfo[] Devices { get; } = TVPlayR.DecklinkIterator.EnumerateDevices();

        public TVPlayR.DecklinkInfo SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _decklink.DeviceIndex = SelectedDevice.Index;
        }

        public override bool IsValid()
        {
            return !(SelectedDevice is null) && string.IsNullOrEmpty(this[nameof(SelectedDevice)]);
        }
    }
}
