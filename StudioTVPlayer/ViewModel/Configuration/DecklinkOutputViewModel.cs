using StudioTVPlayer.Model.Configuration;
using System;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class DecklinkOutputViewModel : OutputViewModelBase
    {
        private DecklinkOutput _decklink;
        private TVPlayR.DecklinkInfo _selectedDevice;
        private TVPlayR.DecklinkKeyer _selectedKeyer;

        public DecklinkOutputViewModel(DecklinkOutput decklink) : base(decklink)
        {
            _decklink = decklink;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == decklink.DeviceIndex);
        }

        public TVPlayR.DecklinkInfo[] Devices => TVPlayR.DecklinkIterator.Devices;

        public Array Keyers { get; } = Enum.GetValues(typeof(TVPlayR.DecklinkKeyer));

        public TVPlayR.DecklinkInfo SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public TVPlayR.DecklinkKeyer SelectedKeyer { get => _selectedKeyer; set => Set(ref _selectedKeyer, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _decklink.DeviceIndex = SelectedDevice.Index;
            _decklink.Keyer = SelectedKeyer;
        }

        public override bool IsValid()
        {
            return !(SelectedDevice is null) && string.IsNullOrEmpty(this[nameof(SelectedDevice)]);
        }

        protected override string ReadErrorInfo(string propertyName)
        {
            switch(propertyName)
            {
                case nameof(SelectedKeyer):
                    if (SelectedDevice?.SupportsKeyer(SelectedKeyer) == false)
                        return $"{SelectedDevice.ModelName} can't support {SelectedKeyer} keyer.";
                    break;
            }
            return base.ReadErrorInfo(propertyName);
        }
    }
}
