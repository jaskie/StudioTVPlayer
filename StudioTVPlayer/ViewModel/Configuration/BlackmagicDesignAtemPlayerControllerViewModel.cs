using System.Collections.Generic;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;

        public BlackmagicDesignAtemPlayerControllerViewModel(Model.BlackmagicDesignAtemDiscovery blackmagicDesignAtemDiscovery, Model.Configuration.BlackmagicDesignAtemPlayerController controllerConfiguration = null)
            : base(controllerConfiguration ?? new Model.Configuration.BlackmagicDesignAtemPlayerController())
        {
            _blackmagicDesignAtemDiscovery = blackmagicDesignAtemDiscovery;
            if (controllerConfiguration != null)
                _selectedDevice = blackmagicDesignAtemDiscovery.Devices.FirstOrDefault(d => d.DeviceId == controllerConfiguration.Id);
        }

        public override void Apply()
        {
            var config = PlayerController as Model.Configuration.BlackmagicDesignAtemPlayerController;
            config.Id = SelectedDevice?.DeviceId;
            config.Name = SelectedDevice?.DeviceName;
            config.Address = SelectedDevice?.Address.ToString();
            config.Port = SelectedDevice?.Port ?? 0;
            IsModified = false;
        }

        public override bool IsValid()
        {
            return SelectedDevice != null;
        }

        internal void NotifyDeviceSeen(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            NotifyPropertyChanged(nameof(Devices));
            if (device.DeviceId == PlayerController.Id)
                SelectedDevice = device;
        }

        internal void NotifyDeviceLost(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            NotifyPropertyChanged(nameof(Devices));
            if (device == _selectedDevice)
                SelectedDevice = null;
        }

        protected override string ReadErrorInfo(string columnName)
        {
            switch(columnName)
            {
                case nameof(SelectedDevice) when SelectedDevice is null:
                    return "Please select switcher";
            }
            return string.Empty;
        }

        public IEnumerable<Model.BlackmagicDesignAtemDeviceInfo> Devices => _blackmagicDesignAtemDiscovery.Devices;

        public Model.BlackmagicDesignAtemDeviceInfo SelectedDevice
        {
            get => _selectedDevice; set
            {
                if (!Set(ref _selectedDevice, value))
                    return;
                NotifyPropertyChanged(nameof(Id));
                NotifyPropertyChanged(nameof(DisplayName));
            }
        }

        public override string Id => SelectedDevice?.DeviceId;

        public override string DisplayName => SelectedDevice is null ? null : $"{SelectedDevice.DeviceName} at {SelectedDevice.Address}";

    }
}
