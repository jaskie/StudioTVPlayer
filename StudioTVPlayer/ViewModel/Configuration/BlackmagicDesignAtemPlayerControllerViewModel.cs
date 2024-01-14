using LibAtem.Net;
using StudioTVPlayer.Model;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private bool _connect;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;
        private AtemClient _atemClient;
        private bool _isConnected;

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
            base.Apply();
        }

        public override bool IsValid()
        {
            return SelectedDevice != null;
        }

        public bool Connect
        {
            get => _connect;
            set
            {
                if (_connect == value)
                    return;
                _connect = value;
                if (value)
                {
                    _atemClient = BlackmagicDesignAtemDevices.GetDevice(Address);
                    _atemClient.OnConnection += OnConnection;
                    _atemClient.OnDisconnect += OnDisconnect;
                }
                else
                {
                    if (BlackmagicDesignAtemDevices.CloseDevice(Address))
                    {
                        _atemClient.OnConnection -= OnConnection;
                        _atemClient.OnDisconnect -= OnDisconnect;
                        _atemClient = null;
                        IsConnected = false;
                    }
                }
                NotifyPropertyChanged();
            }
        }

        private void OnConnection(object _)
        {
            IsConnected = true;
        }

        private void OnDisconnect(object _)
        {
            IsConnected = false;
        }

        public bool IsConnected
        {
            get => _isConnected;
            private set
            {
                if (_isConnected == value)
                    return;
                _isConnected = value;
                NotifyPropertyChanged();
            }
        }

        internal void NotifyDeviceSeen(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            NotifyPropertyChanged(nameof(Devices));
            if (device.DeviceId == PlayerController.Id)
            {
                _selectedDevice = device;
                NotifyPropertyChanged(nameof(SelectedDevice));
            }
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
            return base.ReadErrorInfo(columnName);
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
        public string Address => SelectedDevice?.Address.ToString();

    }
}
