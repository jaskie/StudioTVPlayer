﻿using LibAtem.Commands.MixEffects;
using LibAtem.Common;
using LibAtem.Net;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase, IDisposable
    {
        private BlackmagicDesignAtemDeviceInfoViewModel _selectedDevice;
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
            Devices = new ObservableCollection<BlackmagicDesignAtemDeviceInfoViewModel>(_blackmagicDesignAtemDiscovery.Devices.Select(device => new BlackmagicDesignAtemDeviceInfoViewModel(device)));
            if (controllerConfiguration != null)
            {
                _address = controllerConfiguration.Address;
                _deviceId = controllerConfiguration.DeviceId;
                _selectedDevice = Devices.FirstOrDefault(d => d.DeviceId == controllerConfiguration.DeviceId) ?? Devices.FirstOrDefault(d => d.Address.ToString() == controllerConfiguration.Address);
                foreach (var binging in controllerConfiguration.Bindings.OfType<Model.Configuration.BlackmagicDesignAtemPlayerBinding>())
                    AddBindingViewModel(new BlackmagicDesignAtemPlayerBindingViewModel(binging));
                if (Model.BlackmagicDesignAtemDevices.IsConnected(_address))
                    Connect = true;
            }
            else
                SelectedDevice = Devices.FirstOrDefault();
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
                        _atemClient = Model.BlackmagicDesignAtemDevices.GetDevice(address);
                        _atemClient.OnConnection += OnConnection;
                        _atemClient.OnDisconnect += OnDisconnect;
                        _atemClient.OnReceive += OnReceive;
                        IsConnected = _atemClient.ConnectionVersion != null;
                    }
                    else
                    {
                        if (Model.BlackmagicDesignAtemDevices.ReleaseDevice(address) && _atemClient != null)
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
                        NotifyBindings(Model.BlackmagicDesignAtemCommand.PrgI, prgI.Index, prgI.Source);
                        break;
                    case PreviewInputGetCommand prvI:
                        NotifyBindings(Model.BlackmagicDesignAtemCommand.PrvI, prvI.Index, prvI.Source);
                        break;
                }
            }
        }

        private void NotifyBindings(Model.BlackmagicDesignAtemCommand command, MixEffectBlockId me, VideoSource videoSource)
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

        internal void NotifyDeviceSeen(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            var deviceViewModel = new BlackmagicDesignAtemDeviceInfoViewModel(device);
            Devices.Add(deviceViewModel);
            if (device.DeviceId == _deviceId || device.Address.ToString() == _address)
                SelectedDevice = deviceViewModel;
        }

        internal void NotifyDeviceUpdated(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            var deviceViewModel = Devices.FirstOrDefault(d => d.DeviceId == device.DeviceId || d.Address == device.Address.ToString());
            if (deviceViewModel != null)
            {
                deviceViewModel.Update(device);
                if (deviceViewModel == _selectedDevice)
                    NotifyPropertyChanged(nameof(DisplayName));
            }
        }

        internal void NotifyDeviceLost(Model.BlackmagicDesignAtemDeviceInfo device)
        {
            var deviceViewModel = Devices.FirstOrDefault(d => d.DeviceId == device.DeviceId || d.Address == device.Address.ToString());
            if (deviceViewModel is null)
                return;
            Devices.Remove(deviceViewModel);
            if (deviceViewModel == SelectedDevice)
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

        public ObservableCollection<BlackmagicDesignAtemDeviceInfoViewModel> Devices { get; }

        public BlackmagicDesignAtemDeviceInfoViewModel SelectedDevice
        {
            get => _selectedDevice;
            set
            {
                if (!Set(ref _selectedDevice, value))
                    return;
                _address = value?.Address.ToString();
                _deviceId = value?.DeviceId;
                NotifyPropertyChanged(nameof(DisplayName));
                NotifyPropertyChanged(nameof(CanPressConnect));
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
                    Model.BlackmagicDesignAtemDevices.ReleaseDevice(oldAddress);
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
                Model.BlackmagicDesignAtemDevices.ReleaseDevice(Address);
                _atemClient.OnConnection -= OnConnection;
                _atemClient.OnDisconnect -= OnDisconnect;
                _atemClient.OnReceive -= OnReceive;
                _atemClient = null;
            }
        }
    }
}
