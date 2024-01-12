using System.Collections.Generic;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;

        public BlackmagicDesignAtemPlayerControllerViewModel(Model.Configuration.BlackmagicDesignAtemPlayerController playerController, Model.BlackmagicDesignAtemDiscovery blackmagicDesignAtemDiscovery)
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

        IEnumerable<Model.BlackmagicDesignAtemDeviceInfo> Devices => _blackmagicDesignAtemDiscovery.Devices;

        public Model.BlackmagicDesignAtemDeviceInfo SelectedDevice { get => _selectedDevice; set => Set(ref _selectedDevice, value); }
    }
}
