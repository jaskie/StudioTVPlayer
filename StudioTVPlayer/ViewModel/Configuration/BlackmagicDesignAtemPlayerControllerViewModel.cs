using LibAtem;
using LibAtem.Commands.MixEffects;
using LibAtem.Common;
using LibAtem.Net;
using StudioTVPlayer.Helpers;
using StudioTVPlayer.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Windows.Input;

namespace StudioTVPlayer.ViewModel.Configuration
{
    public sealed class BlackmagicDesignAtemPlayerControllerViewModel : PlayerControllerViewModelBase, IDisposable
    {
        private Model.BlackmagicDesignAtemDeviceInfo _selectedDevice;
        private bool _connect;
        private readonly Model.BlackmagicDesignAtemDiscovery _blackmagicDesignAtemDiscovery;
        private readonly ObservableCollection<BlackmagicDesignAtemPlayerBindingViewModel> _bindings = new ObservableCollection<BlackmagicDesignAtemPlayerBindingViewModel>();
        private AtemClient _atemClient;
        private bool _isConnected;
        private string _address;
        private string _id;
        private bool _disposed;

        public BlackmagicDesignAtemPlayerControllerViewModel(Model.BlackmagicDesignAtemDiscovery blackmagicDesignAtemDiscovery, Model.Configuration.BlackmagicDesignAtemPlayerController controllerConfiguration = null)
            : base(controllerConfiguration ?? new Model.Configuration.BlackmagicDesignAtemPlayerController())
        {
            _blackmagicDesignAtemDiscovery = blackmagicDesignAtemDiscovery;
            if (controllerConfiguration != null)
                _address = controllerConfiguration.Address;
                _id = controllerConfiguration.Id;
                _selectedDevice = blackmagicDesignAtemDiscovery.Devices.FirstOrDefault(d => d.DeviceId == controllerConfiguration.Id) ?? blackmagicDesignAtemDiscovery.Devices.FirstOrDefault(d => d.Address.ToString() == controllerConfiguration.Address);
                foreach (var binging in controllerConfiguration.Bindings.OfType<Model.Configuration.BlackmagicDesignAtemPlayerBinding>())
                {
                    var vm = new BlackmagicDesignAtemPlayerBindingViewModel(binging);
                    vm.Modified += Binding_Modified;
                    vm.RemoveRequested += Binding_RemoveRequested;
                    _bindings.Add(vm);
                }
            AddBindingCommand = new UiCommand(AddBinging);
        }

        public override void Apply()
        {
            var config = PlayerControllerConfiguration as Model.Configuration.BlackmagicDesignAtemPlayerController;
            config.Id = SelectedDevice?.DeviceId;
            config.Name = SelectedDevice?.DeviceName;
            config.Address = Address;
            foreach (var binding in _bindings)
                binding.Apply();
            config.Bindings = _bindings.Select(binding => binding.BingingConfiguration).ToArray();
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
                var address = Address;
                if (!string.IsNullOrEmpty(address))
                {
                    if (value)
                    {
                        _atemClient = BlackmagicDesignAtemDevices.GetDevice(address);
                        _atemClient.OnConnection += OnConnection;
                        _atemClient.OnDisconnect += OnDisconnect;
                        _atemClient.OnReceive += OnReceive;
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

        private void OnReceive(object sender, IReadOnlyList<LibAtem.Commands.ICommand> commands)
        {
            Debug.WriteLine($"{nameof(OnReceive)}: {commands.Count} commands");
            string commandId = null;
            MixEffectBlockId me = default;
            VideoSource source = default;
            foreach (var cmd in commands)
            {
                switch (cmd)
                {
                    case ProgramInputGetCommand prgI:
                        commandId = LibAtem.Commands.CommandManager.FindNameAndVersionForType(prgI).Item1;
                        me = prgI.Index;
                        source = prgI.Source;
                        Debug.WriteLine($"prgI: {prgI.Index}.{prgI.Source}");
                        break;
                    case PreviewInputGetCommand prvI:
                        commandId = LibAtem.Commands.CommandManager.FindNameAndVersionForType(prvI).Item1;
                        me = prvI.Index;
                        source = prvI.Source;
                        Debug.WriteLine($"prvI: {prvI.Index}:{prvI.Source}");
                        break;
                }
            }
            if (Enum.TryParse<BlackmagicDesignAtemCommand>(commandId, out var command))
                foreach (var binding in Bindings)
                    binding.AtemCommandReceived(command, me, source);
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
            NotifyPropertyChanged(nameof(Devices));
            if (device.DeviceId == PlayerControllerConfiguration.Id || device.Address.ToString() == _address)
            {
                _selectedDevice = device;
                _id = device.DeviceId;
                NotifyPropertyChanged(nameof(SelectedDevice));
                NotifyPropertyChanged(nameof(DisplayName));
                NotifyPropertyChanged(nameof(CanPressConnect));
                _address = device.Address.ToString();
                NotifyPropertyChanged(nameof(Address));
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
                NotifyPropertyChanged(nameof(DisplayName));
                NotifyPropertyChanged(nameof(CanPressConnect));
                if (value != null)
                {
                    _address = value.Address.ToString();
                    _id = value.DeviceId;
                }
                NotifyPropertyChanged(nameof(Address));
            }
        }

        public override string Id => _id;
        public override string DisplayName => SelectedDevice is null ? null : $"{SelectedDevice.DeviceName} at {SelectedDevice.Address}";
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

        public ICommand AddBindingCommand { get; }

        public IEnumerable<BlackmagicDesignAtemPlayerBindingViewModel> Bindings => _bindings;
        
        private void AddBinging(object obj)
        {
            var vm = new BlackmagicDesignAtemPlayerBindingViewModel();
            vm.Modified += Binding_Modified;
            vm.RemoveRequested += Binding_RemoveRequested;
            _bindings.Add(vm);
            IsModified = true;
        }

        private void Binding_RemoveRequested(object sender, EventArgs e)
        {
            var vm = sender as BlackmagicDesignAtemPlayerBindingViewModel ?? throw new ArgumentException(nameof(sender));
            vm.Modified -= Binding_Modified;
            vm.RemoveRequested -= Binding_RemoveRequested;
            _bindings.Remove(vm);
        }

        private void Binding_Modified(object sender, EventArgs e)
        {
            IsModified = true;
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
