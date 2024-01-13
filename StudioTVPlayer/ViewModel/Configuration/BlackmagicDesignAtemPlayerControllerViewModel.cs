using System.Collections.Generic;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;

        public BlackmagicDesignAtemPlayerControllerViewModel(Model.BlackmagicDesignAtemDiscovery blackmagicDesignAtemDiscovery, Model.Configuration.BlackmagicDesignAtemPlayerController playerController = null)
            : base(playerController ?? new Model.Configuration.BlackmagicDesignAtemPlayerController())
        {
            _blackmagicDesignAtemDiscovery = blackmagicDesignAtemDiscovery;
        }

        public override void Apply()
        {
            var config = PlayerController as Model.Configuration.BlackmagicDesignAtemPlayerController;
            config.DeviceId = SelectedDevice.DeviceId;
        }

        public override bool IsValid()
        {
            return SelectedDevice != null;
        }

        internal void NotifyDevicesChanged()
        {
            NotifyPropertyChanged(nameof(Devices));
        }

        protected override string ReadErrorInfo(string columnName)
        {
            switch(columnName)
            {
                case nameof(SelectedDevice) when SelectedDevice is null:
                    return "Controlling switcher must be selected";
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
                NotifyPropertyChanged(nameof(Name));
            }
        }

        public override string Id => SelectedDevice?.DeviceId;

        public override string Name => SelectedDevice?.DeviceName;
    }
}
