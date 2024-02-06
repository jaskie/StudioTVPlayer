using LibAtem.Commands.MixEffects;
using LibAtem.Common;
using LibAtem.Net;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase, IDisposable
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private bool _connect;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;
        private AtemClient _atemClient;
        private bool _isConnected;
        private string _address;
        private string _deviceId;
        private bool _disposed;

        public BlackmagicDesignAtemPlayerControllerViewModel(Model.BlackmagicDesignAtemDiscovery blackmagicDesignAtemDiscovery, Model.Configuration.BlackmagicDesignAtemPlayerController controllerConfiguration = null)
            : base(controllerConfiguration ?? new Model.Configuration.BlackmagicDesignAtemPlayerController())
        {
            _blackmagicDesignAtemDiscovery = blackmagicDesignAtemDiscovery;
            if (controllerConfiguration != null)
            {
                _address = controllerConfiguration.Address;
                _deviceId = controllerConfiguration.DeviceId;
                _selectedDevice = blackmagicDesignAtemDiscovery.Devices.FirstOrDefault(d => d.DeviceId == controllerConfiguration.DeviceId) ?? blackmagicDesignAtemDiscovery.Devices.FirstOrDefault(d => d.Address.ToString() == controllerConfiguration.Address);
                foreach (var binging in controllerConfiguration.Bindings.OfType<Model.Configuration.BlackmagicDesignAtemPlayerBinding>())
                    AddBindingViewModel(new BlackmagicDesignAtemPlayerBindingViewModel(binging));
            }
            else
                _selectedDevice = blackmagicDesignAtemDiscovery.Devices.FirstOrDefault();
        }

        public override void Apply()
        {
            var config = PlayerControllerConfiguration as Model.Configuration.BlackmagicDesignAtemPlayerController;
            if (SelectedDevice != null)
            {
                config.DeviceId = SelectedDevice?.DeviceId;
                config.Name = SelectedDevice?.DeviceName;
                config.Address = Address;
            }
            base.Apply();
        }

        public override bool IsValid()
        {
            return !IsModified || (SelectedDevice != null && base.IsValid());
        }

        public bool Connect
        {
            get => _connect;
            set
            {
                if (_connect == value)
                    return;
                _connect = value;
                var address = Address;
                if (!string.IsNullOrEmpty(address))
                {
                    if (value)
                    {
                        _atemClient = BlackmagicDesignAtemDevices.GetDevice(address);
                        _atemClient.OnConnection += OnConnection;
                        _atemClient.OnDisconnect += OnDisconnect;
                        _atemClient.OnReceive += OnReceive;
                        IsConnected = _atemClient.ConnectionVersion != null;
                    }
                    else
                    {
                        if (BlackmagicDesignAtemDevices.ReleaseDevice(address) && _atemClient != null)
                        {
                            _atemClient.OnConnection -= OnConnection;
                            _atemClient.OnDisconnect -= OnDisconnect;
                            _atemClient.OnReceive -= OnReceive;
                            _atemClient = null;
                            IsConnected = false;
                        }
                    }
                }
                else
                    IsConnected = false;
                NotifyPropertyChanged();
            }
        }

        protected override PlayerControllerBindingViewModelBase CreatePlayerControlerBindingViewModel(Model.Configuration.PlayerBindingBase bindingConfiguration = null)
        {
            var blackmagicDesignAtemBindingConfiguration = bindingConfiguration as Model.Configuration.BlackmagicDesignAtemPlayerBinding;
            return new BlackmagicDesignAtemPlayerBindingViewModel(blackmagicDesignAtemBindingConfiguration);
        }

        private void OnReceive(object sender, IReadOnlyList<LibAtem.Commands.ICommand> commands)
        {
            foreach (var cmd in commands)
            {
                switch (cmd)
                {
                    case ProgramInputGetCommand prgI:
                        NotifyBindings(BlackmagicDesignAtemCommand.PrgI, prgI.Index, prgI.Source);
                        break;
                    case PreviewInputGetCommand prvI:
                        NotifyBindings(BlackmagicDesignAtemCommand.PrvI, prvI.Index, prvI.Source);
                        break;
                }
            }
        }

        private void NotifyBindings(BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
        {
            foreach (var binding in Bindings.OfType<BlackmagicDesignAtemPlayerBindingViewModel>())
                binding.AtemCommandReceived(command, me, videoSource);
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

        public bool CanPressConnect => Connect || SelectedDevice != null;

        internal void NotifyDeviceSeen(BlackmagicDesignAtemDeviceInfo device)
        {
            NotifyPropertyChanged(nameof(Devices));
            if (device.DeviceId == _deviceId || device.Address.ToString() == _address)
            {
                _selectedDevice = device;
                NotifyPropertyChanged(nameof(SelectedDevice));
                NotifyPropertyChanged(nameof(DisplayName));
                NotifyPropertyChanged(nameof(CanPressConnect));
                _address = device.Address.ToString();
                NotifyPropertyChanged(nameof(Address));
            }
        }

        internal void NotifyDeviceLost(BlackmagicDesignAtemDeviceInfo device)
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
                NotifyPropertyChanged(nameof(DisplayName));
                NotifyPropertyChanged(nameof(CanPressConnect));
                if (value != null)
                {
                    _address = value.Address.ToString();
                    _deviceId = value.DeviceId;
                }
                NotifyPropertyChanged(nameof(Address));
            }
        }

        public string DeviceId => _deviceId;
        public override string DisplayName => SelectedDevice is null 
            ? (string.IsNullOrEmpty(PlayerControllerConfiguration.Name) ? null : $"{PlayerControllerConfiguration.Name} (not available)")
            : $"{SelectedDevice.DeviceName} at {SelectedDevice.Address}";

        public string Address
        {
            get => _address;
            set
            {
                var oldAddress = _address;
                if (!Set(ref _address, value))
                    return;
                SelectedDevice = null;
                if (IsConnected)
                {
                    if (_atemClient != null)
                    {
                        _atemClient.OnConnection -= OnConnection;
                        _atemClient.OnDisconnect -= OnDisconnect;
                        _atemClient.OnReceive -= OnReceive;
                        _atemClient = null;
                    }
                    BlackmagicDesignAtemDevices.ReleaseDevice(oldAddress);
                    IsConnected = false;
                }
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

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            if (_atemClient != null)
            {
                BlackmagicDesignAtemDevices.ReleaseDevice(Address);
                _atemClient.OnConnection -= OnConnection;
                _atemClient.OnDisconnect -= OnDisconnect;
                _atemClient.OnReceive -= OnReceive;
                _atemClient = null;
            }
        }
    }
}
