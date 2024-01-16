using StudioTVPlayer;
using System;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public class DecklinkOutputViewModel : OutputViewModelBase
    {
        private Model.Configuration.DecklinkOutput _decklinkConfiguration;
        private TVPlayR.DecklinkInfo _selectedDevice;
        private TVPlayR.DecklinkKeyerType _selectedKeyer;
        private TVPlayR.TimecodeOutputSource _selectedTimecodeSource;

        public DecklinkOutputViewModel(Model.Configuration.DecklinkOutput decklinkConfiguration) : base(decklinkConfiguration)
        {
            _decklinkConfiguration = decklinkConfiguration;
            _selectedDevice = Devices.FirstOrDefault(d => d.Index == decklinkConfiguration.DeviceIndex);
            _selectedKeyer = decklinkConfiguration.Keyer;
            _selectedTimecodeSource = decklinkConfiguration.TimecodeSource;
        }

        public TVPlayR.DecklinkInfo[] Devices => TVPlayR.DecklinkIterator.Devices;

        public TVPlayR.DecklinkInfo SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }

        public Array Keyers { get; } = Enum.GetValues(typeof(TVPlayR.DecklinkKeyerType));

        public TVPlayR.DecklinkKeyerType SelectedKeyer { get => _selectedKeyer; set => Set(ref _selectedKeyer, value); }

        public TVPlayR.TimecodeOutputSource SelectedTimecodeSource { get => _selectedTimecodeSource; set => Set(ref _selectedTimecodeSource, value); }

        public override void Apply()
        {
            if (!IsModified)
                return;
            base.Apply();
            _decklinkConfiguration.DeviceIndex = SelectedDevice.Index;
            _decklinkConfiguration.Keyer = SelectedKeyer;
            _decklinkConfiguration.TimecodeSource = SelectedTimecodeSource;
        }

        public override bool IsValid()
        {
            return !(SelectedDevice is null) && string.IsNullOrEmpty(this[nameof(SelectedDevice)]);
        }

        protected override string ReadErrorInfo(string propertyName)
        {
            switch (propertyName)
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
